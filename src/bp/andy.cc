/*
  by andy and neon
 */
#include <cassert>
#include <cmath>
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

const int NUM_INTERMEDIATE = 5;
class AndyPerceptron {
 private:
  std::vector<std::vector<float>> first_layer_weights;
  float                           first_biases[NUM_INTERMEDIATE];
  float                           second_layer_weights[NUM_INTERMEDIATE];
  float                           second_bias;


 public:
  AndyPerceptron() : first_biases{0} {
    this->first_layer_weights = std::vector<std::vector<float>>(
      NUM_INTERMEDIATE, std::vector<float>(HIST_LENGTH, 1.0));
    second_bias = 0;
  }

  float sigmoid(float x) const { return 1 / (1 + exp(-x)); }

  void get_middle_layer(uns32 history, float mem[NUM_INTERMEDIATE]) const {
    for(uns k = 0; k < NUM_INTERMEDIATE; k++) {
      mem[k] = this->first_biases[k];

      for(uns i = 0; i < HIST_LENGTH; i++) {
        int history_bit = (history & (1 << i)) >> i;

        if(history_bit == 0) {
          mem[k] += this->first_layer_weights[k][i] * -1;
        } else {
          mem[k] += this->first_layer_weights[k][i] * 1;
        }
      }
    }

    for(uns k = 0; k < NUM_INTERMEDIATE; k++) {
      mem[k] = sigmoid(mem[k]);
    }
  }
  bool predict(uns32 history) const {
    float middle_layer[NUM_INTERMEDIATE];
    get_middle_layer(history, middle_layer);

    float y = this->second_bias;
    for(uns i = 0; i < NUM_INTERMEDIATE; i++) {
      y += this->second_layer_weights[i] * middle_layer[i];
    }

    return y > 0;
  }

  void update_incorrect(uns32 history) {
    bool predicted_bool = this->predict(history);
    int  correct;
    if(predicted_bool) {
      correct = -1;
    } else {
      correct = 1;
    }

    float middle_layer[NUM_INTERMEDIATE];
    get_middle_layer(history, middle_layer);

    this->second_bias += correct;
    for(uns i = 0; i < NUM_INTERMEDIATE; i++) {
      this->second_layer_weights[i] += correct * (middle_layer[i]);
    }

    for(uns k = 0; k < NUM_INTERMEDIATE; k++) {
      float deriv = middle_layer[k] * (1.0 - middle_layer[k]);
      for(uns i = 0; i < HIST_LENGTH; i++) {
        int   history_bit = (history & (1 << i)) >> i;
        float dir;
        if(history_bit == 0) {
          dir = -1;
        } else {
          dir = 1;
        }
        this->first_layer_weights[k][i] += correct * (deriv * dir * second_layer_weights[k]);
	this ->first_biases[i] += correct * deriv * second_layer_weights[k];
      }
    }
  }
};

namespace {
struct AndyState {
  std::vector<AndyPerceptron> pht;
};
uns32 get_perceptron_index_index(const Addr addr) {
  return addr % NUM_PERCEPTRONS;
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
    gshare_state.pht.resize(NUM_PERCEPTRONS, AndyPerceptron());
  }
}

uns8 bp_andy_pred(Op* op) {
  const uns   proc_id      = op->proc_id;
  const auto& gshare_state = andy_state_all_cores.at(proc_id);

  const Addr           addr      = op->oracle_info.pred_addr;
  const uns32          hist      = op->oracle_info.pred_global_hist;
  const uns32          index     = get_perceptron_index_index(addr);
  const AndyPerceptron pht_entry = gshare_state.pht[index];

  if(pht_entry.predict(hist)) {
    return TAKEN;
  } else {
    return NOT_TAKEN;
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
  const uns32     index     = get_perceptron_index_index(addr);
  AndyPerceptron* pht_entry = &gshare_state.pht[index];


  if(op->oracle_info.mispred) {
    pht_entry->update_incorrect(hist);
  }
}
