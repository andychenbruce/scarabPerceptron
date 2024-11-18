/*
  by andy and neon
 */
#include <cassert>
#include <cstdint>
#include <map>
#include <vector>

#include "andy.h"

extern "C" {
#include "bp/bp.param.h"
#include "core.param.h"
#include "globals/assert.h"
#include "statistics.h"
}


class AndyPerceptron {
 private:
  std::vector<double> weights;
  double              bias;

 public:
  AndyPerceptron() {
    this->weights = std::vector<double>(HIST_LENGTH, 0.0);
    this->bias    = 0;
  }
  bool predict(uns32 history) const {
    double y = this->bias;

    for(uns i = 0; i < HIST_LENGTH; i++) {
      int history_bit = (history & (1 << i)) >> i;

      if(history_bit == 0) {
        y += this->weights[i] * -1;
      } else {
        y += this->weights[i] * 1;
      }
    }

    return y > 0.0;
  }

  void update_incorrect(uns32 history) {
    bool predicted_bool = this->predict(history);
    int  predicted;
    if(predicted_bool) {
      predicted = 1;
    } else {
      predicted = -1;
    }


    this->bias += predicted;
    for(uns i = 0; i < HIST_LENGTH; i++) {
      int history_bit = (history & (1 << i)) >> i;

      this->weights[i] += predicted * history_bit;
    }
  }
};

namespace {
struct AndyState {
  std::vector<AndyPerceptron> pht;
};
uns32 get_pht_index(const Addr addr, const uns32 hist) {
  const uns32 cooked_hist = hist >> (32 - HIST_LENGTH);
  const uns32 cooked_addr = (addr >> 2) & N_BIT_MASK(HIST_LENGTH);
  return cooked_hist ^ cooked_addr;
}
}  // namespace

std::vector<AndyState> andy_state_all_cores;

void bp_andy_timestamp(Op*) {}
void bp_andy_spec_update(Op*) {}
void bp_andy_retire(Op*) {}
void bp_andy_recover(Recovery_Info*) {}
uns8 bp_andy_full(uns) {
  return 0;
}

void bp_andy_init(void) {
  andy_state_all_cores.resize(NUM_CORES);
  for(auto& gshare_state : andy_state_all_cores) {
    gshare_state.pht.resize(1 << HIST_LENGTH, AndyPerceptron());
  }
}

uns8 bp_andy_pred(Op* op) {
  const uns   proc_id      = op->proc_id;
  const auto& gshare_state = andy_state_all_cores.at(proc_id);

  const Addr           addr      = op->oracle_info.pred_addr;
  const uns32          hist      = op->oracle_info.pred_global_hist;
  const uns32          pht_index = get_pht_index(addr, hist);
  const AndyPerceptron pht_entry = gshare_state.pht[pht_index];

  if(pht_entry.predict(hist)) {
    return 1;
  } else {
    return 0;
  }
}

void bp_andy_update(Op* op) {
  if(op->table_info->cf_type != CF_CBR) {
    return;
  }

  const uns proc_id      = op->proc_id;
  auto&     gshare_state = andy_state_all_cores.at(proc_id);

  const Addr      addr      = op->oracle_info.pred_addr;
  const uns32     hist      = op->oracle_info.pred_global_hist;
  const uns32     pht_index = get_pht_index(addr, hist);
  AndyPerceptron* pht_entry = &gshare_state.pht[pht_index];


  if(op->oracle_info.mispred) {
    pht_entry->update_incorrect(hist);
  }
}
