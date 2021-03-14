#define NOMINMAX  // Required on Windows
#include <limits>
#include <cstdlib>
#include <cstdio>
#include "tbb/tbb.h"

#include "kmeans.h"
#include "view.h"
#include "reduce_local_counts_to_global_count.h"
#include "reduce_local_sums_to_global_sum.h"
#include "reduce_min_ind.h"

namespace tbb_asura {

#include "kmeans_tbb.h"

} // namespace tbb_example
