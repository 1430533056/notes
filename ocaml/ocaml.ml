参考: [Partial Evaluation: Staging - 梦开始的地方](https://zhuanlan.zhihu.com/p/643694771)

代码可在线运行：[TryOcaml](https://try.ocamlpro.com/)

type 'a monoid = {
  one: 'a;
  add: 'a -> 'a -> 'a
};;

type 'a semiring = {
  zero: 'a;
  one: 'a;
  add: 'a -> 'a -> 'a;
  mul: 'a -> 'a -> 'a
};;

let add_monoid : int monoid = {
  one = 0;
  add = ( + )
};;

let rec m_b_d (m:'a semiring) (x:'a) (n:int) =
  if n = 0 then m.zero
  else if n mod 2 == 1 then m.add x (m_b_d m x (n-1))
  else m_b_d m (m.add x x)(n / 2)
;;

type expr = 
  | Var of string
  | Int of int
  | Add of expr * expr
  | Mul of expr * expr
  | Let of string * expr * expr
  | Let2 of expr * expr * string
;;

let expr_semiring : expr semiring = {
  zero = Int 0;
  one = Int 1;
  add = (fun x y -> Add(x, y));
  mul = (fun x y -> Mul(x, y));
};;

type letlist = (string * expr) list ref;;

let new_list() : letlist = ref [];;

let ll = new_list();;

let id = ref 0;;

let unique_id() : string =
  let x = !id in id := x + 1; "x"^ string_of_int x
;;

let pushlist (ll:letlist) (e:expr) : expr =
  let id = unique_id() in
  ll := (id, e) :: !ll;
  Var id
;;

let expr_semiring2 (ll:letlist) : expr semiring = {
  zero = Int 0;
  one = Int 1;
  add = (fun x y -> pushlist ll (Add(x,y)));
  mul = (fun x y -> pushlist ll (Mul(x,y)));
};;

let add_semiring (r:expr semiring) : expr monoid = {
  one = r.one;
  add = r.add;
};;

m_b_d (expr_semiring2 ll) (Var "0") 4;; 

ll;; 