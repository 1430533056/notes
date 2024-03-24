#include <stdio.h>
#include <fstream>
#include <sstream>
#include <string>
#include <math.h>
#include <mpi.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>

int rank;
int number_of_processes;
int M;
int N;
int K;

#define printMatrix(ptr, rows, cols)                                      \
do {                                                                      \
    printf("rank:%d %s rows:%d cols:%d\n", rank, #ptr, (rows), (cols));   \
    for (int i = 0; i < (rows); ++i) {                                    \
        for (int j = 0; j < (cols); ++j) {                                \
            printf("%6.2f", ptr[i*(cols) + j]);                           \
        }                                                                 \
        printf("\n");                                                     \
    }                                                                     \
  } while(0);

void initMatrix(float *ptr, int m, int n) {
  for (int i = 0; i < m * n; i++) {
    ptr[i] = i % 5;
  }
}

void gatherMatrix(int mb, int nb, float *loc, int bM, int bN, float *global, MPI_Comm comm) {
    float * A_tmp = NULL;
    if (rank == 0) {
        A_tmp = new float [bM * bN];
    }
    MPI_Gather(loc, mb*nb, MPI_FLOAT, A_tmp, mb*nb, MPI_FLOAT, 0, comm);
    // only rank 0 has something to do, others are free
    if (rank != 0) return;

    int nblks_m = bM / mb;
    int nblks_n = bN / nb;
    int idx = 0;
    for (int blk_i = 0; blk_i < nblks_m; ++blk_i) {
        for (int blk_j = 0; blk_j < nblks_n; ++blk_j) {
            // position in global matrix where current block should start
            int blk_start_row = blk_i * mb;
            int blk_start_col = blk_j * nb;
            for (int i = 0; i < mb; ++i) {
                for (int j = 0; j < nb; ++j) {
                    global[(blk_start_row + i)*bN + (blk_start_col + j)] = A_tmp[idx];
                    idx++;
                }
            }
        }
    }
    // printMatrix(A_tmp, bM, bN);
    free(A_tmp);
}

void scatterMatrix(int m_block, int n_block, int bM, int bN, float *local, float *global,
  MPI_Comm comm, int m, int n, int N, int id = 0) {
  float *temp = nullptr;
  if (rank == 0 || rank == id) {
    // printf("rank:%d\n", rank);
    temp = new float[bM * bN];
    int count = 0;
    for (int i = m; i < m + bM; i += m_block) {
      for (int j = n; j < n + bN; j += n_block) {
        for (int mi = 0; mi < m_block; mi++) {
          for (int nj = 0; nj < n_block; nj++) {
            temp[count++] = global[(i + mi) * N + j + nj];
          }
        }
      }
    }
    // printMatrix(global, bM, bN);
    // printMatrix(temp, bM*bN/m_block/n_block, m_block*n_block);
  }
  MPI_Scatter(temp, m_block * n_block, MPI_FLOAT, local, m_block * n_block, MPI_FLOAT, 0, comm);
  if (rank == 0 || rank == id) {
    delete[] temp;
  }
#if 0
  printMatrix(local, m_block, n_block);
#endif
}

void matMul(int m_block, int k_block, int n_block, float *A_local, float *B_local, float *temp_c) {
  for (int i = 0; i < m_block; i++) {
    for (int j = 0; j < n_block; j++) {
      int temp = 0;
      for (int k = 0; k < k_block; k++) {
        temp += A_local[i*k_block + k] * B_local[k*n_block + j];
      }
      temp_c[i*n_block + j] = temp;
    }
  }
}

void sumMatrix(int m_block, int n_block, float *temp_c, float *C_local) {
  for (int i = 0; i < m_block * n_block; i++) {
    C_local[i] += temp_c[i];
  }
}

void computeDiff(float *C, float *baseline) {
  if (rank != 0) return;
  for (int i = 0; i < M*N; i++) {
    if (std::abs(baseline[i] - C[i]) > 1e-6) {
      std::cout << "\033[31mError at " << i << "\033[0m\n";
      return;
    }
  }
  std::cout << "\033[32mALL Passed!\033[0m\n";
}

int compute(float *C, float *A, float *B, float *C_recv) {
    //Assuming that the processes form a square
    if (number_of_processes != 8) {
        std::cerr << "number_of_processes must be: " << 8 << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    int procs_row = 2;
    int procs_col = number_of_processes / procs_row;
    if (rank == 0) {
        std::cout << "rows: " << procs_row << " " << "cols: " << procs_col << "\n";
    }
    int n_dims = 2;
    int dims[n_dims] = {procs_row, procs_col};
    int periods[n_dims] = {0, 0};
    int repeat = 0;

    //create comm_groups
    MPI_Comm comm_cart;
    MPI_Cart_create(MPI_COMM_WORLD, n_dims, dims, periods, repeat, &comm_cart);

    int bM = M;
    int bN = N;
    int bK = K;
    int n_bM = M / bM;
    int n_bN = N / bN;
    int n_bK = K / bK;

    int m_block = bM / procs_row;
    int n_block = bN / procs_col;
    int k_split = procs_row < procs_col ? procs_row : procs_col;
    int k_block = bK / k_split;
    int bcast = bK / k_block;

    int other_id = 0;
    if (procs_col != procs_row) {
      other_id = k_split == procs_col ? k_split * k_split : k_split;
    }

    if (m_block * procs_row != bM) {
        std::cerr << "m must be dividable by n_procs_row" << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    if (n_block * procs_col != bN) {
        std::cerr << "n must be dividable by n_procs_col" << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    if (k_block * k_split != bK) {
        std::cerr << "k must be dividable by n_procs_col" << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    float * A_local = (float *) calloc(m_block * k_block, sizeof(float));
    float * B_local = (float *) calloc(k_block * n_block, sizeof(float));
    float * C_local = (float *) calloc(m_block * n_block, sizeof(float));

    float * A_saved = (float *) calloc(m_block * k_block, sizeof(float));
    float * B_saved = (float *) calloc(k_block * n_block, sizeof(float));
    float * C_tmp =   (float *) calloc(m_block * n_block, sizeof(float));

    float *C_buf = new float[bM * bN];
    memset(C_local, 0x0, m_block * n_block * sizeof(float));

    int coords[2];
    MPI_Cart_coords(comm_cart, rank, 2, coords);
    const int my_row = coords[0];
    const int my_col = coords[1];

    MPI_Comm groups1;
    MPI_Comm_split(comm_cart, int(my_row < k_split && my_col < k_split), rank, &groups1);
    MPI_Comm groups2 = MPI_COMM_WORLD;
    if (procs_col < procs_row) {
      groups1 = MPI_COMM_WORLD;
      MPI_Comm_split(comm_cart, int(my_row < k_split && my_col < k_split), rank, &groups2);
    }

    MPI_Comm row_comm, col_comm;
    MPI_Comm_split(comm_cart, my_col, rank, &col_comm);
    MPI_Comm_split(comm_cart, my_row, rank, &row_comm);

    for (int k = 0; k < K; k += bK) {
      // printf("rank:%d\n", rank);
      MPI_Barrier(MPI_COMM_WORLD);

      scatterMatrix(m_block, k_block, bM, bK, A_local, A, groups1, 0, k, K, other_id);
      scatterMatrix(k_block, n_block, bK, bN, B_local, B, groups2, k, 0, N, other_id);

      memcpy(A_saved, A_local, m_block * k_block * sizeof(float));
      memcpy(B_saved, B_local, k_block * n_block * sizeof(float));

      if (rank == 4) {
        // printMatrix(A_local, m_block, k_block);
        // printMatrix(B_local, k_block, n_block);
      }

      for(int broadcaster = 0; broadcaster < bcast; ++broadcaster) {
        if (my_col == broadcaster) {
            memcpy(A_local, A_saved, m_block * k_block * sizeof(float));
        }
        MPI_Bcast(A_local, m_block * k_block, MPI_FLOAT, broadcaster, row_comm);

        if (my_row  == broadcaster) {
            memcpy(B_local, B_saved, k_block * n_block * sizeof(float));
        }

        MPI_Bcast(B_local, n_block * k_block, MPI_FLOAT, broadcaster, col_comm);

        matMul(m_block, k_block, n_block, A_local, B_local, C_tmp);
        
        sumMatrix(m_block, n_block, C_tmp, C_local);
        if (rank == 14) {
          // printMatrix(A_local, m_block, k_block);
          // printMatrix(B_local, k_block, n_block);
          // printMatrix(C_local, m_block, n_block);
        }
      }
    }
    gatherMatrix(m_block, n_block, C_local, bM, bN, C, MPI_COMM_WORLD);
    delete[] C_buf;
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &number_of_processes);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    M = 10; K = 14; N = 16;

    if (rank == 0) {
      std::cout << "======================" << std::endl;
      std::cout << "M: " << M << ", K: " << K << ", N: " << N << std::endl;
    }

    float * A = new float[M * K];
    float * B = new float[K * N];
    float * C = new float[M * N];
    float * C_recv = new float[M * N];
    float * baseline = new float[M * N];
    initMatrix(A, M, K);
    initMatrix(B, K, N);
    if (rank == 0) {
      // printMatrix(A, M, K);
      // printMatrix(B, K, N);
    }
    compute(C, A, B, C_recv);
    matMul(M, K, N, A, B, baseline);
    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0) {
      // printMatrix(C, M, N);
      // printMatrix(baseline, M, N);
      computeDiff(C, baseline);
    }
    delete[] A;
    delete[] B;
    delete[] C;
    delete[] C_recv;
    delete[] baseline;
    MPI_Finalize();
}

// run cmd
// mpicxx summa.cpp -o summa
// mpirun -np 8 ./summa
// https://juejin.cn/post/7171997964465307655