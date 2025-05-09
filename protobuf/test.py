from google.protobuf import text_format
from proto import test_pb2

f = open("test.prototxt", 'r')
graph = test_pb2.Graph()
text_format.Parse(f.read(), graph)
f.close()
print(graph)

graph = test_pb2.Graph()
tensor = test_pb2.Tensor()
tensor2 = test_pb2.Tensor()
tensor.name = "Tesnor1"
tensor.value_f.extend([1,2,3,4])
tensor2.name = "Tesnor2"
tensor2.value_f.extend([1,2,3])
graph.name = "graph"
graph.inputs.append(tensor)
graph.outputs.append(tensor2)
print(text_format.MessageToString(graph))