// See LICENSE for license details.
package designutils

import chisel3._
import chisel3.util._

/** A rich counter module
 *
  * @param n number of counts before the counter resets (or one more than the
  * @param saturate true if saturates at max value
  * @param initVal init value of the counter
  * maximum output value of the counter), need not be a power of two
  */
class RichCounter(val n: BigInt, val saturate: Boolean = false, val initVal: Int = 0) {
  require(n >= 0)
  val value = if (n > 1) RegInit(initVal.U(log2Ceil(n).W)) else 0.U

  /** Increment the counter, returning whether the counter currently is at the
    * maximum and will wrap. The incremented value is registered and will be
    * visible on the next cycle.
    */
  def inc(): Bool = {
    if (n > 1) {
      val wrap = value === (n-1).asUInt
      value := value + 1.U
      if(saturate) {
        when (wrap) { value := (n-1).U }
      } else if (!isPow2(n)) {
        when (wrap) { value := 0.U }
      }
      wrap
    } else {
      true.B
    }
  }
}
object RichCounter
{
  /** Instantiate a [[Counter! counter]] with the specified number of counts.
    */
  def apply(n: BigInt, sat: Boolean = false): RichCounter = new RichCounter(n, sat)

  /** Instantiate a [[Counter! counter]] with the specified number of counts and a gate.
   *
    * @param cond condition that controls whether the counter increments this cycle
    * @param n number of counts before the counter resets
    * @return tuple of the counter value and whether the counter will wrap (the value is at
    * maximum and the condition is true).
    *
    * @example {{{
    * val countOn = true.B // increment counter every clock cycle
    * val (counterValue, counterWrap) = Counter(countOn, 4)
    * when (counterValue === 3.U) {
    *   ...
    * }
    * }}}
    */
  def apply(cond: Bool, n: BigInt, sat: Boolean): (UInt, Bool) = {
    val c = new RichCounter(n, sat)
    var wrap: Bool = null
    when (cond) { wrap = c.inc() }
    (c.value, cond && wrap)
  }
}

