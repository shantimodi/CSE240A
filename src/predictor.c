//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "Shanti Modi";
const char *studentID   = "A53305577";
const char *email       = "shmodi@eng.ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//

uint32_t global_history_register;
uint32_t gShare_BHT_size = pow(2,13); //given
uint32_t gShareBHTCurrentIndex;
vector<uint8_t> gShareBHT;


//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void
init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //
  gShareBHT(gShare_BHT_size, 3); //Initializing to strongly taken
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{
  //
  //TODO: Implement prediction scheme
  //
  uint32_t gShareBHTCurrentIndex = pc ^ (global_history_register & (pow(2,ghistoryBits) - 1)) % gShare_BHT_size ;
  
  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
      if(gShareBHT[gShareBHTCurrentIndex] > 1)       
       return TAKEN;
    case TOURNAMENT:
    case CUSTOM:
    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void
train_predictor(uint32_t pc, uint8_t outcome)
{
  //
  //TODO: Implement Predictor training
  //
  uint8_t previous_value = gShareBHT[gShareBHTCurrentIndex];
  uint8_t new_value = outcome ? ((previous_value != 3) ? (previous_value + 1) : 3) : ( (previous_value != 0) ? (previous_value - 1) : 0));
  
  gShareBHT[gShareBHTCurrentIndex] = new_value;
  global_history_register<<=1;
  global_history_register |= outcome;
  
  }
