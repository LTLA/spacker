#' Pack small positive integers
#'
#' Pack those small positive integers with a combination of variable-bit and run-length encoding.
#'
#' @param x A vector of small positive integers.
#' @param method String specifying the type of packing to perform.
#' 
#' @return A raw vector containing the packed integers.
#'
#' @author Aaron Lun
#'
#' @examples
#' y <- rpois(1000, lambda=0.5) + 1
#'
#' out <- spack(y)
#' length(out)
#'
#' out2 <- spack(y, method="multiplier4")
#' length(out2)
#'
#' @export
spack <- function(x, method=c("doubling", "multiplier4")) {
    method <- match.arg(method)
    if (method == "doubling") {
        return(doubling_packer(x))
    } else {
        return(multiplier_packer(x))
    }
}
