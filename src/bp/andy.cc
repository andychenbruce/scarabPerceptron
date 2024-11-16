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


void bp_andy_timestamp(Op*) {}
void bp_andy_spec_update(Op*) {}
void bp_andy_retire(Op*) {}
void bp_andy_recover(Recovery_Info*) {}
uns8 bp_andy_full(uns) {
  return 0;
}

class AndyPerceptron{
private:
  std::vector<double> weights;
  double bias;
public:
  AndyPerceptron(){
    this->weights = std::vector<double>(HIST_LENGTH, 0.0);
    this->bias = 0;
  }
  bool predict(uns32 history){
    double y = this->bias;

    for(uns i = 0; i < HIST_LENGTH; i++){
      int history_bit = (history & (1 << i)) >> i;

      if(history_bit == 0){
	y += this->weights[i] * -1;
      }else{
	y += this->weights[i] * 1;
      }
    }
    
    return y > 0.0;
  }
};

std::map<Addr, AndyPerceptron> perceptron_table;

void bp_andy_init(void) {}

uns8 bp_andy_pred(Op* op) {
  Addr addr = op->oracle_info.va;

  if(perceptron_table.find(addr) != perceptron_table.end()){
    AndyPerceptron* p = &perceptron_table[addr];
    if (p->predict(op->oracle_info.pred_global_hist)){
      return 1;
    }else{
      return 0;
    }
  }else{
    AndyPerceptron new_p{};
    perceptron_table[addr] = new_p;
    if(new_p.predict(op->oracle_info.pred_global_hist)){
      return 1;
    }else{
      return 0;
    }
  }
}
void bp_andy_update(Op*) {
  assert(false);
}
