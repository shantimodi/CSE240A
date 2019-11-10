//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"

#define ONE 0x1

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

uint32_t gshare_BHT_size = 0x0;
uint32_t local_PHT_size  = 0x0;
uint32_t local_BHT_size  = 0x0;
uint32_t global_BHT_size = 0x0;
uint32_t chooser_size   = 0x0;
uint32_t custom_BHT_size = 0x0;

uint8_t *gshare_BHT;
uint32_t *local_PHT;
uint8_t *local_BHT;
uint8_t *global_BHT;
uint8_t *chooser;
uint8_t *custom_BHT;

uint32_t gshare_BHT_current_Index;
uint32_t local_PHT_current_Index;
uint32_t local_BHT_current_Index;
uint32_t global_BHT_current_Index;
uint32_t custom_BHT_current_Index;

uint8_t local_prediction;
uint8_t global_prediction;
uint8_t chooser_prediction;
uint8_t custom_prediction;

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
  
   gshare_BHT_size = ONE<<ghistoryBits; 
   local_PHT_size = ONE<<pcIndexBits;
   local_BHT_size = ONE<<lhistoryBits;
   global_BHT_size = ONE<<ghistoryBits;
   chooser_size = ONE<<ghistoryBits;
   custom_BHT_size = ONE<<13;
   
   gshare_BHT = (uint8_t *)calloc(gshare_BHT_size, sizeof(uint8_t));
   for(int i=0; i<gshare_BHT_size; i++)
     gshare_BHT[i]=0x01;
   local_PHT = calloc(local_PHT_size, sizeof(uint32_t));
   local_BHT = calloc(local_BHT_size, sizeof(uint8_t));
   for(int i=0; i<local_BHT_size; i++)
     local_BHT[i]=0x01;
   global_BHT = calloc(global_BHT_size, sizeof(uint8_t));
   for(int i=0; i<global_BHT_size; i++)
     global_BHT[i]=0x01;
   chooser = calloc(chooser_size, sizeof(uint8_t));
   for(int i=0; i<chooser_size; i++)
     chooser[i]=0x10;
   custom_BHT = calloc(custom_BHT_size, sizeof(uint8_t));	 
   for(int i=0; i<custom_BHT_size; i++)
     custom_BHT[i]=0x01;
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
  
  uint32_t local_BHT_32_bit_Index = 0;

  // Make a prediction based on the bpType
  switch (bpType) {
    
	case STATIC:
      return TAKEN;
	  
    case GSHARE:
	  gshare_BHT_current_Index = (pc ^ (global_history_register & (uint32_t)((ONE<< ghistoryBits)-1)))% gshare_BHT_size;
      if(gshare_BHT[gshare_BHT_current_Index] > 1)       
       return TAKEN;
   
    case TOURNAMENT:
	  local_PHT_current_Index = pc & (uint32_t)((ONE<<pcIndexBits)-1);
	  local_BHT_32_bit_Index = local_PHT[local_PHT_current_Index];
	  local_BHT_current_Index = (local_BHT_32_bit_Index) & (uint32_t)((ONE<<lhistoryBits)-1);
	  local_prediction = local_BHT[local_BHT_current_Index];
	  
	  global_BHT_current_Index = global_history_register & (uint32_t)((ONE<<ghistoryBits)-1);
	  global_prediction = global_BHT[global_BHT_current_Index];
	  chooser_prediction = chooser[global_BHT_current_Index];
	
	  if(chooser_prediction > 1)
		  return (global_prediction > 1 ? TAKEN : NOTTAKEN);
	  return (local_prediction > 1 ? TAKEN : NOTTAKEN);
	  
    case CUSTOM:
	 custom_BHT_current_Index = ((pc>>2) ^ (global_history_register & (uint32_t)((ONE<< 13)-1)))%gshare_BHT_size;
	 custom_prediction = (custom_BHT[custom_BHT_current_Index] >> (pc%4)*2) & 3; 
	 return (custom_prediction > 1 ? TAKEN : NOTTAKEN);
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
  
  uint8_t previous_value = 0;
  uint8_t new_value, new_value_2 = 0;
  uint16_t counter = 0;
  uint8_t is_global_correct=0;
  
  switch (bpType) {
    
    case GSHARE:
      previous_value = gshare_BHT[gshare_BHT_current_Index];
      new_value = outcome ? ((previous_value != 3) ? (previous_value + 1) : 3) : ( (previous_value != 0) ? (previous_value - 1) : 0);
      gshare_BHT[gshare_BHT_current_Index] = new_value;
	  break;
     
    case TOURNAMENT:	 
	
      // training chooser
      if((local_prediction>>1) != (global_prediction>>1))
      {
	     is_global_correct = (outcome == ((global_prediction)>>1)) ? 1 : 0;
	     
	     previous_value = chooser[global_BHT_current_Index];
	     new_value = is_global_correct ? ((previous_value !=3) ? (previous_value + 1) : 3) : ( (previous_value !=0) ? (previous_value - 1) : 0);
	     chooser[global_BHT_current_Index] = new_value;
	     
      }
	  
	  //training local_predictor
	      
      previous_value = local_prediction;
      new_value = outcome ? ((previous_value != 3) ? (previous_value + 1) : 3) : ( (previous_value != 0) ? (previous_value - 1) : 0);
      local_BHT[local_BHT_current_Index] = new_value;
      
	  
	  //training global predictor
	  
      previous_value = global_prediction;
      new_value = outcome ? ((previous_value != 3) ? (previous_value + 1) : 3) : ( (previous_value != 0) ? (previous_value - 1) : 0);
      global_BHT[global_BHT_current_Index] = new_value;
      
      //updating PHT
      local_PHT[local_PHT_current_Index]<<=1;
      local_PHT[local_PHT_current_Index] |= outcome;
	  break;
	  
	case CUSTOM:
	  previous_value = custom_prediction;
	  new_value_2 = outcome ? ((previous_value != 3) ? (previous_value + 1) : 3) : ( (previous_value != 0) ? (previous_value - 1) : 0);  
	  
	  switch(pc%4)
	  {
	  case 0 : 
	    new_value = (custom_BHT[custom_BHT_current_Index] & 0x11111100) | new_value_2;
	    break;       
	  case 1 : 
	    new_value = (custom_BHT[custom_BHT_current_Index] & 0x11110011) | (new_value_2 << 2);
		break;
	  case 2 : 
	    new_value = (custom_BHT[custom_BHT_current_Index] & 0x11001111) | (new_value_2 << 4);
		break;
	  case 3 : 
	    new_value = (custom_BHT[custom_BHT_current_Index] & 0x00111111) | (new_value_2 << 6);
		break;
	  default:
        break;
	  }
	  custom_BHT[custom_BHT_current_Index] = new_value;
	  break;
	  
    default:
      break;	
  }
  
  // updating global history register
  global_history_register<<=1;
  global_history_register |= outcome;
  
  }
