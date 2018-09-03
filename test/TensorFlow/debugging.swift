// RUN: %target-swift-frontend -Xllvm -tf-dump-intermediates -emit-sil %s | %FileCheck %s

import TensorFlow

public func basicDebugValues(_ x: Tensor<Float>) {
  let y = x + 1
  let z = y.squared()
}

// FIXME: `debug_value_addr` for `z` is not currently preserved due to SSA promotion in deabstraction.
public func debugValuesInLoop(_ x: Tensor<Float>, _ n: Int) {
  var z = x.squared()
  for i in 0..<n {
    z += x
  }
}

// Opaque handles should not trigger send/receive even at -Onone, so we won't
// expect their `debug_value` to work.
public func noCopyForOpaqueHandles() {
  let values = Tensor<Float>([1.0])
  let dataset: VariantHandle =
    #tfop("TensorSliceDataset", [values],
          Toutput_types: [Float.self], output_shapes: [TensorShape()])
}

// CHECK-LABEL: --- TFPartition Accelerator Result: {{.*}}basicDebugValues
// CHECK: @{{.*}}basicDebugValues{{.*}}.tf
// CHECK: [[ONE:%.*]] = graph_op "Const"
// CHECK: [[ADD_RESULT:%.*]] = graph_op "Add,i,i"
// CHECK: graph_op "Square,i"([[ADD_RESULT]] : $TensorHandle<Float>) {T: $Float, __device: "/device:CPU:0"} : $TensorHandle<Float>


// CHECK-LABEL: --- TFPartition Accelerator Result: {{.*}}debugValuesInLoop
// CHECK: bb0
// CHECK:   [[SQUARED:%.*]] = graph_op "Square,i"
// CHECK-NEXT:   br bb1([[SQUARED]] : $TensorHandle<Float>)
// CHECK: bb1
// CHECK:   graph_op "tfc.RecvFromHost"()
// CHECK:   [[COND:%.*]] = graph_op "tf_tensor_to_i1"
// CHECK:   cond_br [[COND]], bb2, bb3
// CHECK: bb2:
// CHECK:   [[ADD_RESULT:%.*]] = graph_op "Add,i,i"
// CHECK-NEXT:   br bb1([[ADD_RESULT]]


// CHECK-LABEL: ---- PARTITION STATE FOR FUNCTION ${{.*}}noCopyForOpaqueHandle
// CHECK: result values:
// CHECK-NOT: graph_op "TensorSliceDataset,L,e"(%4 : $TensorHandle<Float>)


// CHECK-LABEL: --- TFPartition Host Result: {{.*}}basicDebugValues
// CHECK: debug_value %{{.*}} : $Tensor<Float>, let, name "x", argno 1
// CHECK: debug_value %{{.*}} : $Tensor<Float>, let, name "y"
// CHECK: debug_value %{{.*}} : $Tensor<Float>, let, name "z"


// CHECK-LABEL: --- TFPartition Host Result: {{.*}}debugValuesInLoop{{.*}}
// CHECK: bb0(%0 : $Tensor<Float>):
// CHECK:   debug_value %{{.*}} : $Tensor<Float>, let, name "x", argno 1
// CHECK:   debug_value %{{.*}} : $Int, let, name "i"