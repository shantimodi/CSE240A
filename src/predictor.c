//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"
#include <math.h>
#include <stdlib.h>


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
uint32_t gShare_BHT_size = (64 * (2<<10)) >> BYTE_VALUE; //given
uint32_t local_BHT_size = (2<<(lhistoryBits+1))>> BYTE_VALUE;
uint32_t global_BHT_size = (2<<(ghistoryBits+1)) >> BYTE_VALUE;
uint32_t chooser_size = (2<<(ghistoryBits+1)) >> BYTE_VALUE;


/*** Assuming meaningful combination of lhistoryBits, pcIndexBits, ghistoryBits are given
  ** Always taking Global predictor to be non-aliasing and local_BHT to be non-aliasing **
     Assuming we only have 14kb space for Tournament predictor.
	 TODO : Split 14kb buffer with proper allocation if not meaningful combinations are given
***/
uint32_t local_PHT_size = ((14 * (2<<10)) - local_BHT_size - global_BHT_size - chooser_size)>> BYTE_VALUE;
uint32_t gShareBHTCurrentIndex;
uint16_t local_PHT_current_Index;
uint8_t *gShareBHT;
uint8_t *local_PHT;
uint8_t *local_BHT;
uint8_t *global_BHT;
uint8_t *chooser;


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
  gShareBHT = (uint8_t *)calloc(gShare_BHT_size, sizeof(uint8_t)); //Initializing to strongly not taken
  local_PHT = (uint8_t *)calloc(local_PHT_size, sizeof(uint8_t)); //Initializing to strongly not taken
  local_BHT = (uint8_t *)calloc(local_BHT_size, sizeof(uint8_t));
  global_BHT = (uint8_t *)calloc(global_PHT_size, sizeof(uint8_t)); //Initializing to strongly not taken
  chooser = (uint8_t *)calloc(chooser_size, sizeof(uint8_t)); //Initializing to strongly not taken
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
  gShareBHTCurrentIndex = (pc ^ (global_history_register & (uint32_t)((2<<ghistoryBits) - 1))) % gShare_BHT_size ;
  local_PHT_current_Index = (pc & (uint32_t)((2<<pcIndexBits) -1)) % local_BHT_size ; 
  uint16_t local_BHT_16_bits = local_BHT[(local_PHT_current_Index*lhistoryBits)/8] + local_BHT[(local_PHT_current_Index*lhistoryBits)/8 + 1] << 8; 
  uint16_t local_BHT_n_bits_mask = ((2<<lhistoryBits) - 1) << ((local_PHT_current_Index*lhistoryBits) % 8);
  uint16_t local_BHT_Index = (local_BHT_16_bits & local_BHT_n_bits_mask) >> ((local_PHT_current_Index*lhistoryBits) % 8);
  uint8_t local_prediction_8_bits = local_BHT[local_BHT_Index >> (lhistoryBits-8)];
  uint8_t local_prediction_2_bits_mask =  3 << ((local_BHT_Index & 3)*2);
  uint8_t local_prediction = (local_prediction_8_bits & local_prediction_2_bits_mask) >> ((local_BHT_Index & 3)*2);
  
  global_BHT_Current_Index = (global_history_register & (uint32_t)((2<<ghistoryBits) - 1)) % global_BHT_size;
  uint8_t global_BHT_2_bits_mask =  3 << ((global_BHT_Current_Index & 3)*2);
  
  
  global_BHT_8_bits = global_BHT[global_BHT_Current_Index >> (ghistoryBits-8)];
  uint8_t global_prediction = (global_BHT_8_bits & global_BHT_2_bits_mask) >> ((global_BHT_Current_Index & 3)*2);
  
  chooser_8_bits = chooser[global_BHT_Current_Index >> (ghistoryBits-8)];
  uint8_t chooser_prediction = (chooser_8_bits & global_BHT_2_bits_mask) >> ((global_BHT_Current_Index & 3)*2);
  
  
  
  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
      if(gShareBHT[gShareBHTCurrentIndex] > 1)       
       return TAKEN;
    case TOURNAMENT:
	   if(chooser_prediction > 1)
		   return global_prediction > 1 ? TAKEN : NOTTAKEN;
       else
           return local_prediction > 1 ? TAKEN : NOTTAKEN;		   
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
  uint8_t new_value = outcome ? ((previous_value != 3) ? (previous_value + 1) : 3) : ( (previous_value != 0) ? (previous_value - 1) : 0);
  
  gShareBHT[gShareBHTCurrentIndex] = new_value;
  global_history_register<<=1;
  global_history_register |= outcome;
  
  }
