import chisel3._
import chisel3.util._

package object trws {
  val tree_dims = Array( 1, 1, 1, 1, 1, 1)
//  val tree_dims = Array( 2, 1, 1, 1, 1, 1)
//  val tree_dims = Array( 2, 2, 1, 1, 1, 1)
//  val tree_dims = Array( 2, 2, 2, 1, 1, 1)
//  val tree_dims = Array( 2, 2, 2, 2, 1, 1)
//  val tree_dims = Array( 2, 2, 2, 2, 2, 1)
  val ctree_dims = tree_dims.scanLeft(1){ _ * _} drop 1
  assert( chisel3.util.isPow2( ctree_dims.last))
  val log2_elements_per_cl = chisel3.util.log2Floor( ctree_dims.last)
  val elements_per_cl = 1<<log2_elements_per_cl
  val bitwidth = 16
  val radixPoint = 10
  val bits_per_cl = bitwidth*elements_per_cl
  val log2_max_cl_per_row = 2
  val max_cl_per_row = 1<<log2_max_cl_per_row
  val cl_per_row = 3
  val max_n = max_cl_per_row*elements_per_cl
  val n = cl_per_row*elements_per_cl
  val n_slices = n*(n-1)/2
  val log2_ncontexts = 1
  val ncontexts = 1<<log2_ncontexts
  def SSIMD() = Vec( elements_per_cl, SInt(bitwidth.W))
  def USIMD() = Vec( elements_per_cl, UInt(bitwidth.W))
  assert( cl_per_row <= max_cl_per_row)
  assert( n_slices > 0)
}
