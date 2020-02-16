#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<stdbool.h>
#define ROB_SIZE 32
#define IQ_SIZE 16
#define LSQ_SIZE 32

#include "cpu.h"

/* Set this flag to 1 to enable debug messages */
#define ENABLE_DEBUG_MESSAGES 1

/*
 * This function creates and initializes APEX cpu.
 */
APEX_CPU*
APEX_cpu_init(const char* filename)
{
  if (!filename)
   {
    return NULL;
   }

  APEX_CPU* cpu = malloc(sizeof(*cpu));
  if (!cpu)
   {
    return NULL;
   }

  /* Initialize PC, Registers and all pipeline stages */
  cpu->pc = 4000;
  memset(cpu->regs, 0, sizeof(int) * 16);
  memset(cpu->regs_valid, 1, sizeof(int) * 16);
  memset(cpu->stage, 0, sizeof(CPU_Stage) * NUM_STAGES);
  memset(cpu->data_memory, 0, sizeof(int) * 4000);

  cpu->stage[EX].flush=0;

  for (int i =0; i<16; i++) 
  {
    cpu->regs_valid[i] = 1;
    cpu->buff_valid[i] = 1;
  }

  /* Parse input file and create code memory */
  cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);

  if (!cpu->code_memory) 
  {
    free(cpu);
    return NULL;
  }

  if (ENABLE_DEBUG_MESSAGES) 
  {
    fprintf(stderr, "APEX_CPU : Initialized APEX CPU, loaded %d instructions\n", cpu->code_memory_size);
    fprintf(stderr, "APEX_CPU : Printing Code Memory\n");
    printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode", "rd", "rs1", "rs2", "imm");

    for (int i = 0; i < cpu->code_memory_size; ++i) 
    {
      printf("%-9s %-9d %-9d %-9d %-9d\n",
             cpu->code_memory[i].opcode,
             cpu->code_memory[i].rd,
             cpu->code_memory[i].rs1,
             cpu->code_memory[i].rs2,
             cpu->code_memory[i].imm);
    }
}

  /* Make all stages busy except Fetch stage, initally to start the pipeline */
  for (int i = 1; i < NUM_STAGES; ++i) 
  {
    cpu->stage[i].busy = 1;
  }
  return cpu;
}

/*
 * This function de-allocates APEX cpu.
 *
 * Note : You are free to edit this function according to your
 *        implementation
 */
void APEX_cpu_stop(APEX_CPU* cpu)
{
  free(cpu->code_memory);
  free(cpu);
}

/* Converts the PC(4000 series) into
 * array index for code memory
 *
 * Note : You are not supposed to edit this function
 *
 */
int get_code_index(int pc)
{
  return (pc - 4000) / 4;
}

static void print_instruction(CPU_Stage* stage)
{
  if (strcmp(stage->opcode, "STORE") == 0)
   {
    printf("%s,R%d,R%d,#%d ", stage->opcode, stage->rs1, stage->rs2, stage->imm);
   }

   if (strcmp(stage->opcode, "LOAD") == 0) 
  {
    printf("%s,R%d,R%d,#%d ", stage->opcode, stage->rd, stage->rs1, stage->imm);
  }

  if (strcmp(stage->opcode, "MOVC") == 0) 
  {
    printf("%s,R%d,#%d ", stage->opcode, stage->rd, stage->imm);
  }
  
  if (strcmp(stage->opcode, "ADD") == 0) 
  {
    printf("%s,R%d,R%d,R%d", stage->opcode, stage->rd, stage->rs1,stage->rs2);
  }
  
  if (strcmp(stage->opcode, "SUB") == 0) 
  {
    printf("%s,R%d,R%d,R%d", stage->opcode, stage->rd, stage->rs1,stage->rs2);
  }
  
  if (strcmp(stage->opcode, "AND") == 0) 
  {
    printf("%s,R%d,R%d,R%d", stage->opcode, stage->rd, stage->rs1,stage->rs2);
  }
  
  if (strcmp(stage->opcode, "OR") == 0) 
  {
    printf("%s,R%d,R%d,R%d", stage->opcode, stage->rd, stage->rs1,stage->rs2);
  }
  
  if (strcmp(stage->opcode, "XOR") == 0) 
  {
    printf("%s,R%d,R%d,R%d", stage->opcode, stage->rd, stage->rs1,stage->rs2);
  }
  
  if (strcmp(stage->opcode, "MUL") == 0) 
  {
    printf("%s,R%d,R%d,R%d", stage->opcode, stage->rd, stage->rs1,stage->rs2);
  }
  
  if (strcmp(stage->opcode, "BZ") == 0) 
  {
    printf("%s,#%d", stage->opcode, stage->imm);
  }
  
  if (strcmp(stage->opcode, "BNZ") == 0) 
  {
    printf("%s,#%d", stage->opcode, stage->imm);
  }
  
  if (strcmp(stage->opcode, "JUMP") == 0) 
  {
    printf("%s,R%d,#%d", stage->opcode,stage->rs1,stage->imm);
  }
  
  if (strcmp(stage->opcode, "HALT") == 0)
   {
    printf("%s", stage->opcode);
   }

   if (strcmp(stage->opcode, "") == 0)
   {
    printf("EMPTY");
   }
}

/* Debug function which dumps the cpu stage
 * content
 */
static void print_stage_content(char* name, CPU_Stage* stage)
{
  printf("%-15s: pc(%d) ", name, stage->pc);
  print_instruction(stage);
  printf("\n");
}

/*
 *  Fetch Stage of APEX Pipeline implementation
 */
int fetch(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[F];
  if(cpu->stage[EX].flush==1)
  {
     strcpy(cpu->stage[F].opcode, "");
     printf("Fetch         : EMPTY\n");    
  }
  
 else if (!stage->busy && !stage->stalled) 
  {  
    stage->pc = cpu->pc;

    /* Index into code memory using this pc and copy all instruction fields into
     * fetch latch
     */
    APEX_Instruction* current_ins = &cpu->code_memory[get_code_index(cpu->pc)];
    strcpy(stage->opcode, current_ins->opcode);
    stage->rd = current_ins->rd;
    stage->rs1 = current_ins->rs1;
    stage->rs2 = current_ins->rs2;
    stage->imm = current_ins->imm;
    stage->rd = current_ins->rd;

  if(!cpu->stage[DRF].stalled)
  {
    /* Update PC for next instruction */
    cpu->pc += 4;

    /* Copy data from fetch latch to decode latch*/
    cpu->stage[DRF] = cpu->stage[F];
  }

    if (ENABLE_DEBUG_MESSAGES)
    {
      print_stage_content("Fetch", stage);
    }
  }
  
  else if(stage->stalled==1 || stage->busy==1)                // To stall fetch when MUL enters Execute stage
  {
    /* Store current PC in fetch latch */
    stage->pc = cpu->pc;

    /* Index into code memory using this pc and copy all instruction fields into
     * fetch latch
     */
    APEX_Instruction* current_ins = &cpu->code_memory[get_code_index(cpu->pc)];
    strcpy(stage->opcode, current_ins->opcode);
    stage->rd = current_ins->rd;
    stage->rs1 = current_ins->rs1;
    stage->rs2 = current_ins->rs2;
    stage->imm = current_ins->imm;
    stage->rd = current_ins->rd;
    
   if(ENABLE_DEBUG_MESSAGES)
    {
      print_stage_content("Fetch", stage);
    }
  }
  return 0;
}

/*
 *  Decode Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 *         implementation
 */
int decode(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[DRF];
  if(stage->stalled) 
  {
    stage->stalled = 0;
  }
  if(cpu->stage[EX].flush==1)
  {
     strcpy(cpu->stage[F].opcode, "");
     printf("Decode        : EMPTY\n");    
  }
    
  else if (!stage->busy && !stage->stalled) 
  {
    //STORE
    if (strcmp(stage->opcode, "STORE") == 0) 
    {
    	//when load is in mem stage then free store in DRF that is unstall and validate.

      stage->arithmetic_instr = 0;
      //printf("\njyrfgfvhkfuvhkufgvrk: %s\n",cpu->stage[EX].opcode);
      if(strcmp(cpu->stage[EX].opcode,"LOAD")==0 && (cpu->stage[EX].rd==stage->rs1 || cpu->stage[EX].rd==stage->rs2)) {
      cpu->stage[F].stalled=1; 
      cpu->stage[DRF].stalled=1;
  }else{
    if(cpu->regs_valid[stage->rs1] && cpu->regs_valid[stage->rs2])        //Valid bits of regs checked for dependency
    {
        cpu->stage[F].stalled=0;
        cpu->stage[DRF].stalled=0;
        stage->rs1_value= cpu->regs[stage->rs1];
        stage->rs2_value= cpu->regs[stage->rs2];
    }
    else if(!cpu->regs_valid[stage->rs1]  && cpu->regs_valid[stage->rs2])
    {
        cpu->stage[F].stalled=0;
        cpu->stage[DRF].stalled=0;
        stage->rs1_value=cpu->buff[stage->rs1];  
        stage->rs2_value=cpu->regs[stage->rs2]; 
        cpu->regs_valid[stage->rd]--;
     /* cpu->stage[F].stalled=1; 
      cpu->stage[DRF].stalled=1;*/
    }

    else  if(cpu->regs_valid[stage->rs1]  && !cpu->regs_valid[stage->rs2] )
    {
    	 cpu->stage[F].stalled=0;
        cpu->stage[DRF].stalled=0;
        stage->rs1_value=cpu->regs[stage->rs1];  
        stage->rs2_value=cpu->buff[stage->rs2]; 
        cpu->regs_valid[stage->rd]--; 
    }

    else  if(!cpu->regs_valid[stage->rs1]  && !cpu->regs_valid[stage->rs2] )
    {
    	 cpu->stage[F].stalled=0;
        cpu->stage[DRF].stalled=0;
        stage->rs1_value=cpu->buff[stage->rs1];  
        stage->rs2_value=cpu->buff[stage->rs2]; 
        cpu->buff_valid[stage->rd]--; 
    }
    else
    {
      cpu->stage[F].stalled=1; 
      cpu->stage[DRF].stalled=1;
    }}
  
    /*stage->rs1_value= cpu->regs[stage->rs1];
    stage->rs2_value= cpu->regs[stage->rs2];*/

    }
  
  // LOAD
  if (strcmp(stage->opcode, "LOAD") == 0) 
  {
    stage->arithmetic_instr = 0;
    if(strcmp(cpu->stage[EX].opcode,"LOAD")==0 && (cpu->stage[EX].rd==stage->rs1 || cpu->stage[EX].rd==stage->rs2)) {
      cpu->stage[F].stalled=1; 
      cpu->stage[DRF].stalled=1;
  }else{
    if(cpu->regs_valid[stage->rs1])       
    {
      cpu->stage[F].stalled=0; 
      cpu->stage[DRF].stalled=0;
      stage->rs1_value= cpu->regs[stage->rs1];                
      cpu->regs_valid[stage->rd]--; 
      printf("%d\n", cpu->regs_valid[stage->rd]);
    }

    else if(cpu->buff_valid[stage->rs1])       
    {
      cpu->stage[F].stalled=0; 
      cpu->stage[DRF].stalled=0;
      stage->rs1_value= cpu->buff[stage->rs1];
      cpu->buff_valid[stage->rd]--;
    }

    else
    {
      cpu->stage[F].stalled=1; 
      cpu->stage[DRF].stalled=1;
    }
       }   
  }

  //JUMP
    if (strcmp(stage->opcode, "JUMP") == 0) 
    {
      stage->arithmetic_instr = 0;
      if(cpu->regs_valid[stage->rs1]) {
      	stage->rs1_value= cpu->regs[stage->rs1];
  	  } else {
  	  	stage->rs1_value=cpu->buff[stage->rs1];
  	  }
    }

    //MOVC
    if (strcmp(stage->opcode, "MOVC") == 0)
     {
      stage->arithmetic_instr = 0;                        // 0 is invalid for dependency
      cpu->regs_valid[stage->rd]--;           
    }
  
  // ADD
  if (strcmp(stage->opcode, "ADD") == 0) 
  {
    stage->arithmetic_instr = 1;
    if(strcmp(cpu->stage[EX].opcode,"LOAD")==0 && (cpu->stage[EX].rd==stage->rs1 || cpu->stage[EX].rd==stage->rs2)) {
      cpu->stage[F].stalled=1; 
      cpu->stage[DRF].stalled=1;
  }else{
    if(cpu->regs_valid[stage->rs1] && cpu->regs_valid[stage->rs2])  
    {
      cpu->stage[F].stalled=0; 
      cpu->stage[DRF].stalled=0;
      stage->rs1_value= cpu->regs[stage->rs1];
      stage->rs2_value= cpu->regs[stage->rs2];
      cpu->regs_valid[stage->rd]--;
    }

    else  if(cpu->buff_valid[stage->rs1]  && cpu->regs_valid[stage->rs2] )
    {
    	cpu->stage[F].stalled=0; 
        cpu->stage[DRF].stalled=0;
        stage->rs1_value=cpu->buff[stage->rs1];  
        stage->rs2_value=cpu->regs[stage->rs2]; 
        cpu->regs_valid[stage->rd]--;
    }

    else  if(cpu->regs_valid[stage->rs1]  && cpu->buff_valid[stage->rs2] )
    {
    	cpu->stage[F].stalled=0; 
      cpu->stage[DRF].stalled=0;
        stage->rs1_value=cpu->regs[stage->rs1];  
        stage->rs2_value=cpu->buff[stage->rs2]; 
        cpu->regs_valid[stage->rd]--; 
    }

    else  if(cpu->buff_valid[stage->rs1]  && cpu->buff_valid[stage->rs2] )
    {
    	cpu->stage[F].stalled=0; 
      cpu->stage[DRF].stalled=0;
        stage->rs1_value=cpu->buff[stage->rs1];  
        stage->rs2_value=cpu->buff[stage->rs2]; 
        cpu->regs_valid[stage->rd]--; 
    }

    else
    {
      cpu->stage[F].stalled=1; 
      cpu->stage[DRF].stalled=1;
    }
    }
    /*stage->rs1_value= cpu->regs[stage->rs1];
    stage->rs2_value= cpu->regs[stage->rs2];
    cpu->regs_valid[stage->rd]--; */
  }
  
  // SUB
  if (strcmp(stage->opcode, "SUB") == 0) 
  {
    stage->arithmetic_instr = 1;
    if(strcmp(cpu->stage[EX].opcode,"LOAD")==0 && (cpu->stage[EX].rd==stage->rs1 || cpu->stage[EX].rd==stage->rs2)) {
      cpu->stage[F].stalled=1; 
      cpu->stage[DRF].stalled=1;
  }else{
    if(cpu->regs_valid[stage->rs1] && cpu->regs_valid[stage->rs2])  
    {
      cpu->stage[F].stalled=0; 
      cpu->stage[DRF].stalled=0;
      stage->rs1_value= cpu->regs[stage->rs1];
      stage->rs2_value= cpu->regs[stage->rs2];
      cpu->regs_valid[stage->rd]--;
    }

    else  if(cpu->buff_valid[stage->rs1]  && cpu->regs_valid[stage->rs2])
    {
    	cpu->stage[F].stalled=0; 
      cpu->stage[DRF].stalled=0;
       stage->rs1_value=cpu->buff[stage->rs1];  
       stage->rs2_value=cpu->regs[stage->rs2]; 
       cpu->regs_valid[stage->rd]--;
    }

    else  if(cpu->regs_valid[stage->rs1]  && cpu->buff_valid[stage->rs2])
    {
    	cpu->stage[F].stalled=0; 
      cpu->stage[DRF].stalled=0;
        stage->rs1_value=cpu->regs[stage->rs1];  
        stage->rs2_value=cpu->buff[stage->rs2]; 
        cpu->regs_valid[stage->rd]--; 
    }

    else  if(cpu->buff_valid[stage->rs1]  && cpu->buff_valid[stage->rs2])
    {
    	cpu->stage[F].stalled=0; 
      cpu->stage[DRF].stalled=0;
          stage->rs1_value=cpu->buff[stage->rs1];  
          stage->rs2_value=cpu->buff[stage->rs2]; 
          cpu->regs_valid[stage->rd]--; 
    }
    else
    {
      cpu->stage[F].stalled=1; 
      cpu->stage[DRF].stalled=1;
    }
}
    /*stage->rs1_value= cpu->regs[stage->rs1];
    stage->rs2_value= cpu->regs[stage->rs2];
    cpu->regs_valid[stage->rd]--; */
    }
  
  // AND
  if (strcmp(stage->opcode, "AND") == 0) 
  {
    stage->arithmetic_instr = 0;
    if(strcmp(cpu->stage[EX].opcode,"LOAD")==0 && (cpu->stage[EX].rd==stage->rs1 || cpu->stage[EX].rd==stage->rs2)) {
      cpu->stage[F].stalled=1; 
      cpu->stage[DRF].stalled=1;
  }else{
    if(cpu->regs_valid[stage->rs1] && cpu->regs_valid[stage->rs2])  
    {
      cpu->stage[F].stalled=0; 
      cpu->stage[DRF].stalled=0;
      stage->rs1_value= cpu->regs[stage->rs1];
      stage->rs2_value= cpu->regs[stage->rs2];
      cpu->regs_valid[stage->rd]--;
    }

    else  if(cpu->buff_valid[stage->rs1]  && cpu->regs_valid[stage->rs2])
    {
    	cpu->stage[F].stalled=0; 
      cpu->stage[DRF].stalled=0;
      stage->rs1_value=cpu->buff[stage->rs1];  
      stage->rs2_value=cpu->regs[stage->rs2]; 
      cpu->regs_valid[stage->rd]--;
    }

    else  if(cpu->regs_valid[stage->rs1]  && cpu->buff_valid[stage->rs2])
    {
    	cpu->stage[F].stalled=0; 
      cpu->stage[DRF].stalled=0;
      stage->rs1_value=cpu->regs[stage->rs1];  
      stage->rs2_value=cpu->buff[stage->rs2]; 
      cpu->regs_valid[stage->rd]--; 
    }

    else  if(cpu->buff_valid[stage->rs1]  && cpu->buff_valid[stage->rs2])
    {
    	cpu->stage[F].stalled=0; 
      cpu->stage[DRF].stalled=0;
      stage->rs1_value=cpu->buff[stage->rs1];  
      stage->rs2_value=cpu->buff[stage->rs2]; 
      cpu->regs_valid[stage->rd]--; 
    }
    else
    {
      cpu->stage[F].stalled=1; 
      cpu->stage[DRF].stalled=1;
    }  
}
   /* stage->rs1_value= cpu->regs[stage->rs1];
    stage->rs2_value= cpu->regs[stage->rs2];
    cpu->regs_valid[stage->rd]--; */
    }
  
  // OR
  if (strcmp(stage->opcode, "OR") == 0) 
  {
    stage->arithmetic_instr = 0;
    if(strcmp(cpu->stage[EX].opcode,"LOAD")==0 && (cpu->stage[EX].rd==stage->rs1 || cpu->stage[EX].rd==stage->rs2)) {
      cpu->stage[F].stalled=1; 
      cpu->stage[DRF].stalled=1;
  }else{
    if(cpu->regs_valid[stage->rs1] && cpu->regs_valid[stage->rs2])  
    {
      cpu->stage[F].stalled=0; 
      cpu->stage[DRF].stalled=0;
      stage->rs1_value= cpu->regs[stage->rs1];
      stage->rs2_value= cpu->regs[stage->rs2];
      cpu->regs_valid[stage->rd]--;
    }

    else  if(cpu->buff_valid[stage->rs1]  && cpu->regs_valid[stage->rs2])
    {
    	cpu->stage[F].stalled=0; 
      cpu->stage[DRF].stalled=0;
      stage->rs1_value=cpu->buff[stage->rs1];  
      stage->rs2_value=cpu->regs[stage->rs2]; 
      cpu->regs_valid[stage->rd]--; 
    }

    else  if(cpu->regs_valid[stage->rs1]  && cpu->buff_valid[stage->rs2])
    {
    	cpu->stage[F].stalled=0; 
      cpu->stage[DRF].stalled=0;
      stage->rs1_value=cpu->regs[stage->rs1];  
      stage->rs2_value=cpu->buff[stage->rs2]; 
      cpu->regs_valid[stage->rd]--; 
    }

   else  if(!cpu->regs_valid[stage->rs1]  && !cpu->regs_valid[stage->rs2])
   {
   	cpu->stage[F].stalled=0; 
      cpu->stage[DRF].stalled=0;
      stage->rs1_value=cpu->buff[stage->rs1];  
      stage->rs2_value=cpu->buff[stage->rs2]; 
      cpu->regs_valid[stage->rd]--; 
    }
    else
    {
      cpu->stage[F].stalled=1; 
      cpu->stage[DRF].stalled=1;
    } }
    }
  
  // XOR
  if (strcmp(stage->opcode, "XOR") == 0) 
  {
    stage->arithmetic_instr = 0;
    if(strcmp(cpu->stage[EX].opcode,"LOAD")==0 && (cpu->stage[EX].rd==stage->rs1 || cpu->stage[EX].rd==stage->rs2)) {
      cpu->stage[F].stalled=1; 
      cpu->stage[DRF].stalled=1;
  }else{
    if(cpu->regs_valid[stage->rs1] && cpu->regs_valid[stage->rs2])                // For Dependency
    {
      cpu->stage[F].stalled=0; 
      cpu->stage[DRF].stalled=0;
      stage->rs1_value= cpu->regs[stage->rs1];
      stage->rs2_value= cpu->regs[stage->rs2];
      cpu->regs_valid[stage->rd]--;  
    }
     else  if(cpu->buff_valid[stage->rs1]  && cpu->regs_valid[stage->rs2])
    {
    	cpu->stage[F].stalled=0; 
      cpu->stage[DRF].stalled=0;
      stage->rs1_value=cpu->buff[stage->rs1];  
      stage->rs2_value=cpu->regs[stage->rs2]; 
      cpu->regs_valid[stage->rd]--; 
    }

    else  if(cpu->regs_valid[stage->rs1]  && cpu->buff_valid[stage->rs2])
    {
    	cpu->stage[F].stalled=0; 
      cpu->stage[DRF].stalled=0;
      stage->rs1_value=cpu->regs[stage->rs1];  
      stage->rs2_value=cpu->buff[stage->rs2]; 
      cpu->regs_valid[stage->rd]--; 
    }

   else  if(cpu->buff_valid[stage->rs1]  && cpu->buff_valid[stage->rs2])
   {
   	cpu->stage[F].stalled=0; 
      cpu->stage[DRF].stalled=0;
      stage->rs1_value=cpu->buff[stage->rs1];  
      stage->rs2_value=cpu->buff[stage->rs2]; 
      cpu->regs_valid[stage->rd]--; 
    }

    else
    {
      cpu->stage[F].stalled=1; 
      cpu->stage[DRF].stalled=1;
    }
}
      /*stage->rs1_value= cpu->regs[stage->rs1];
      stage->rs2_value= cpu->regs[stage->rs2];
      cpu->regs_valid[stage->rd]--; */
    }
  
  // MUL
  if (strcmp(stage->opcode, "MUL") == 0) 
  {
    stage->arithmetic_instr = 1;
    if(strcmp(cpu->stage[EX].opcode,"LOAD")==0 && (cpu->stage[EX].rd==stage->rs1 || cpu->stage[EX].rd==stage->rs2)) {
      cpu->stage[F].stalled=1; 
      cpu->stage[DRF].stalled=1;
  }else{
    if(cpu->regs_valid[stage->rs1] && cpu->regs_valid[stage->rs2])  
    {
      cpu->stage[F].stalled=0; 
      cpu->stage[DRF].stalled=0;
      stage->rs1_value= cpu->regs[stage->rs1];
      stage->rs2_value= cpu->regs[stage->rs2];
      cpu->regs_valid[stage->rd]--;  
    }
     else  if(cpu->buff_valid[stage->rs1]  && cpu->regs_valid[stage->rs2])
    {
    	cpu->stage[F].stalled=0; 
      cpu->stage[DRF].stalled=0;
      stage->rs1_value=cpu->buff[stage->rs1];  
      stage->rs2_value=cpu->regs[stage->rs2]; 
      cpu->regs_valid[stage->rd]--; 
    }

    else  if(cpu->regs_valid[stage->rs1]  && cpu->buff_valid[stage->rs2])
    {
    	cpu->stage[F].stalled=0; 
      cpu->stage[DRF].stalled=0;
      stage->rs1_value=cpu->regs[stage->rs1];  
      stage->rs2_value=cpu->buff[stage->rs2]; 
      cpu->regs_valid[stage->rd]--; 
    }

   else  if(cpu->buff_valid[stage->rs1]  && cpu->buff_valid[stage->rs2])
   {
   	  cpu->stage[F].stalled=0; 
      cpu->stage[DRF].stalled=0;
      stage->rs1_value=cpu->buff[stage->rs1];  
      stage->rs2_value=cpu->buff[stage->rs2]; 
      cpu->regs_valid[stage->rd]--; 
   }

    else
    {
      cpu->stage[F].stalled=1; 
      cpu->stage[DRF].stalled=1;
    }
}
    /*stage->rs1_value= cpu->regs[stage->rs1];
    stage->rs2_value= cpu->regs[stage->rs2];
    cpu->regs_valid[stage->rd]--; */
    }

     //HALT
    if(strcmp(stage->opcode, "HALT") == 0) 
    {
    if(!strcmp(rob.entry[front].operation,"HALT") && rob.tag[front]=='c' && !strcmp(me.instruction_info.instruction.instruction_string," ")){
            dequeue_rob();
            cpu->halt=1;}
        stage->arithmetic_instr = 0;
        cpu->stage[F].stalled = 1;
        cpu->stage[F].pc = 0;
        strcpy(cpu->stage[F].opcode, "");
        cpu->ex_halt = 1;
    }
    if(!strcmp(rob.entry[front].operation,"HALT"))
            rob.tag[front] = 'c';

    //BZ and BNZ
    if(strcmp(stage->opcode, "BZ") == 0 || strcmp(stage->opcode, "BNZ") == 0) 
    {
      stage->arithmetic_instr = 0;
        if((cpu->stage[WB].arithmetic_instr == 1) || (cpu->stage[MEM].arithmetic_instr == 1)) 
        {
         stage->stalled = 1;
        } 
        else
        {
        stage->stalled = 0;
        }
    }


    /* Copy data from decode latch to execute latch*/
    cpu->stage[EX] = cpu->stage[DRF];

    if(ENABLE_DEBUG_MESSAGES)
    {
      print_stage_content("Decode/RF", stage);
  }
  
  }
  
  else
   {  
   if(ENABLE_DEBUG_MESSAGES)
   {
      print_stage_content("Decode/RF", stage);
     }
  } 
  return 0;
}

void dispatch_and_issue(){
  if(!iq_full()){ //ONCE JUMP IS DISPATCHED, FLUSH DRF AND FETCH AND STOP FETCHING BCUZ NO POINT
    if(stage_will_write(&d) & !d.stalled){
      d.instruction_info.cod = s.cycle +1;
      if(!strcmp(d.instruction_info.operation,"HALT")){
        PC = 9999;
        stage_init(&f);
        goto NO_MORE;
      }
      if(!strcmp(d.instruction_info.operation,"LOAD")||!strcmp(d.instruction_info.operation,"STORE")){
        enqueue_lsq(&d);
      }

      if(!strcmp(d.instruction_info.operation,"BZ") || !strcmp(d.instruction_info.operation,"BNZ") || !strcmp(d.instruction_info.operation,"JUMP") || !strcmp(d.instruction_info.operation,"JAL")){
        d.instruction_info.dest.zero.status = false;
        enqueue_cfq(&d);
      }

      if(is_arthmetic(&d)){
        d.instruction_info.dest.zero.status = false;
        for(int i=0;i<=31;i++){
          if(!strcmp(d.instruction_info.dest.name,prf.P[i].name))
            prf.P[i].zero = d.instruction_info.dest.zero;
        }
      }
      enqueue_iq(&d); 
      NO_MORE:
      enqueue_rob(&d);  
      ins_init(&d.instruction_info);
      dv=0;
      dfetched=false;
      flagfetched=0;
    }
   //  read_broadcasted_reg_lsq();
    // read_broadcasted_reg_iq();
  }
}
struct InstructionInfo get_ins_iq(int i){
  return iq.ins[i];
}

struct InstructionInfo get_int_from_iq(){
  struct InstructionInfo i;
  for(int i=0;i<=IQ_SIZE-1;i++){
    if(!strcmp(iq.ins[i].operation,"BZ") || !strcmp(iq.ins[i].operation,"BNZ")){
      if(iq.ins[i].dest.zero.status && !iq.ins[i].issued){
        iq.ins[i].issued = true;
        return get_ins_iq(i);
      }
      else goto E;
    }
    if(iq.ins[i].src1.status && iq.ins[i].src2.status && iq_has_int(i) && !iq.ins[i].issued){
      iq.ins[i].issued = true;
      return get_ins_iq(i);
    }
    struct InstructionInfo get_mul_from_iq(){
  struct InstructionInfo i;
  for(int i=0;i<=IQ_SIZE-1;i++){
    if(!strcmp(iq.ins[i].operation,"BZ") || !strcmp(iq.ins[i].operation,"BNZ"))
      goto E;

    if(iq.ins[i].src1.status && iq.ins[i].src2.status && iq_has_mul(i) && !iq.ins[i].issued){
      // return dequeue_iq(i);
      iq.ins[i].issued = true;
      return get_ins_iq(i);
    }
  E:
  ;
  }
  ins_init(&i);
  return i;
}

struct InstructionInfo get_div_from_iq(){
  struct InstructionInfo i;
  for(int i=0;i<=IQ_SIZE-1;i++){
    if(!strcmp(iq.ins[i].operation,"BZ") || !strcmp(iq.ins[i].operation,"BNZ")) // BRANCH IS FOUND, IGNORE AND GOTO NEXT INSTRUCTIONS
      continue;
    if(iq.ins[i].src1.status && iq.ins[i].src2.status && iq_has_div(i) &&!iq.ins[i].issued){
      iq.ins[i].issued = true;
      return get_ins_iq(i);
    }
  }
  ins_init(&i);
  return i;
}
void forward_data_to_iq(struct Stage* from, struct Queue* to){
  for(int i=0;i<=IQ_SIZE-1;i++){
    if(!strcmp(to->ins[i].src1.name,from->instruction_info.dest.name)){
      to->ins[i].src1.value = from->instruction_info.dest.value;
      to->ins[i].src1.status = true;
    }
    if(!strcmp(to->ins[i].src2.name,from->instruction_info.dest.name)){
      to->ins[i].src2.value = from->instruction_info.dest.value;
      to->ins[i].src2.status = true;
    }
  }
}

void forward_data_to_lsq(struct Stage* from, struct Queue* to){
  for(int i=0;i<=LSQ_SIZE-1;i++){
    if(!strcmp(to->ins[i].src1.name,from->instruction_info.dest.name)){
      to->ins[i].src1.value = from->instruction_info.dest.value;
      to->ins[i].src1.status = true;
    }
    if(!strcmp(to->ins[i].src2.name,from->instruction_info.dest.name)){
      to->ins[i].src2.value = from->instruction_info.dest.value;
      to->ins[i].src2.status = true;
    }
  }
}
void load_bypass(struct InstructionInfo* ins){
  int place=-1;
  int top;
  int load=-1;
  struct InstructionInfo temp;
  for(int i=0;i<=LSQ_SIZE-1;i++){
    if(lsq.ins[i].cod == ins->cod)
      place = i;
  }
  if(place!=-1){
  temp = lsq.ins[place];
  for(int i=place-1;i>=0;i--){
    if(!strcmp(lsq.ins[i].operation,"LOAD") && lsq.ins[i].target_address!=-1){
      load = i;
    }
  }
  if(load!=-1){
    for(int i=place;i>load;i--){
      lsq.ins[i] = lsq.ins[i-1];
    }
    lsq.ins[load+1] = temp;
  }
  else{
    top = 0;
    for(int k=place;k>top;k--){
      lsq.ins[k] = lsq.ins[k-1];
    }
    lsq.ins[top] = temp;
  }
}
}

void load_bypass1(int x,struct InstructionInfo* ins){
  ins->dest.value = lsq.ins[x].src1.value;
  for(int i=0;i<=ROB_SIZE-1;i++){
    if(rob.entry[i].cod == ins->cod){
      rob.entry[i].dest = ins->dest;
      rob.tag[i] = 'c'; 
    }
  }
  for(int i=0;i<=31;i++){
    if(!strcmp(prf.P[i].name,ins->dest.name))
      prf.P[i] = ins->dest;
  }
  for(int i=0;i<LSQ_SIZE-1;i++){
    if(!strcmp(ins->dest.name,lsq.ins[i].src1.name))
      lsq.ins[i].src1=ins->dest;
    if(!strcmp(ins->dest.name,lsq.ins[i].src2.name))
      lsq.ins[i].src2=ins->dest;
  }
  for(int i=0;i<IQ_SIZE-1;i++){
    if(!strcmp(ins->dest.name,iq.ins[i].src1.name))
      iq.ins[i].src1=ins->dest;
    if(!strcmp(ins->dest.name,iq.ins[i].src2.name))
      iq.ins[i].src2=ins->dest;
  }
  if(!strcmp(ins->dest.name,d.instruction_info.src1.name))
    d.instruction_info.src1 = ins->dest;
  if(!strcmp(ins->dest.name,d.instruction_info.src2.name))
    d.instruction_info.src2 = ins->dest;
  dequeue_lsq(ins);
}
void update_from_mem(){   //  UPDATE FROM MEM STAGE TO PRF, ROB AND UPDATE PRECISE STATE IF IT WAS BEFORE ANY BRANCH
  for(int i=0;i<=ROB_SIZE-1;i++){
    if(me.instruction_info.cod == rob.entry[i].cod){
      rob.entry[i].dest.status=true;
      rob.entry[i].dest.value = me.instruction_info.dest.value;
      rob.tag[i]='c';
    }
  }
  void update_rob_tag(struct InstructionInfo* ins){
  for(int i=0;i<=ROB_SIZE-1;i++){
    if(rob.entry[i].cod == ins->cod){
      rob.tag[i]='e';
      return;
    }
  }
}
void read_broadcasted_reg_iq(){
  for(int i=0;i<=IQ_SIZE-1;i++){
    for(int j=0;j<=31;j++){
      if(!strcmp(iq.ins[i].src1.name,prf.P[j].name))
        iq.ins[i].src1=prf.P[j];

      if(!strcmp(iq.ins[i].src2.name,prf.P[j].name))
        iq.ins[i].src2=prf.P[j];
    }
  }
}

void read_broadcasted_reg_lsq(){
  for(int i=0;i<=LSQ_SIZE-1;i++){
    for(int j=0;j<=31;j++){
      if(!strcmp(lsq.ins[i].src1.name,prf.P[j].name))
        lsq.ins[i].src1=prf.P[j];

      if(!strcmp(lsq.ins[i].src2.name,prf.P[j].name))
        lsq.ins[i].src2=prf.P[j];
    }
  }

}
for(int i=0;i<=ROB_SIZE-1;i++){

    if(in.instruction_info.cod == rob.entry[i].cod && (!strcmp(in.instruction_info.operation,"JAL") || !strcmp(in.instruction_info.operation,"JUMP"))){ //IF INT WAS JUMP OR JAL
      rob.entry[i].dest = in.instruction_info.dest;
      rob.tag[i]='c';
      }

    if(in.instruction_info.cod == rob.entry[i].cod && !strcmp(in.instruction_info.operation,"BZ")){ //IF INT WAS BZ
      rob.entry[i].dest = in.instruction_info.dest;
      dequeue_cfq(in.instruction_info.cod);
      rob.tag[i]='c';
    }

    if(in.instruction_info.cod == rob.entry[i].cod && !strcmp(in.instruction_info.operation,"BNZ")){  //IF INT WAS BNZ
      rob.entry[i].dest = in.instruction_info.dest;
      dequeue_cfq(in.instruction_info.cod);
      rob.tag[i]='c';
    }
  }

  if((!strcmp(in.instruction_info.operation,"STORE") || !strcmp(in.instruction_info.operation,"LOAD")) && in.instruction_info.target_address!=-1 ){   //IF INTFU HAS LOAD OR STORE AND ADDRESS HAS BEEN CALCULATED
    in.instruction_info.issued = false;
    for(int i=0;i<=31;i++){
      if(rob.entry[i].cod == in.instruction_info.cod){
        if(rob.tag[i]!='c')
        rob.tag[i] = 'w';
      }
    }
    for(int i=0;i<=31;i++){
        if(lsq.ins[i].cod == in.instruction_info.cod)
          lsq.ins[i]=in.instruction_info;
      }
    dequeue_iq(&in.instruction_info);
  }
}
for(int j=0;j<=ROB_SIZE-1;j++){
      if(rob.entry[j].cod == b_cod && b_cod!=0){
        st = j;
      }
    }
    if(st!=0){
      int max = arthmetic_cod_i(&rob.entry[st-1]);
      for(int j=st-2;j>=0;j--){
        if(arthmetic_cod_i(&rob.entry[j]) > max)
          max=arthmetic_cod_i(&rob.entry[j]);
      }
      // ALRDY_THERE:
      if(max!=0){
        for(int k=0;k<=ROB_SIZE-1;k++){
          if(rob.entry[k].cod == max && rob.tag[k] == 'c'){
            iq.ins[i].dest.zero = rob.entry[k].dest.zero;
            iq.ins[i].dest.zero.status = true;
            return;
          }
        }
      }
      //  ROB FUNCTIONS

void commit_to_arf(){   //  TO COMMIT FROM HEAD OF ROB TO ARCHITECTURAL REGISTER FILE
  for(int k=0;k<=31;k++){
    if(!strcmp(prf.P[k].name,rob.entry[front].dest.name) && strcmp(rob.entry[front].operation,"STORE")){
      prf.P[k] = rob.entry[front].dest;
      for(int i=0;i<=15;i++){
        if(!strcmp(prf.renamed[k],rf.R[i].name)){
          rf.R[i].value =prf.P[k].value;
          if(is_arithmetic_i(&rob.entry[front]))
            rf.R[i].zero.bit = prf.P[k].zero.bit; ///////////////////REVIEW
        }
      }
      break;
    }
  }
  if(is_arithmetic_i(&rob.entry[front]))
    last_arthmetic = rob.entry[front];
}
void dequeue_cfq(int cod){
  if(cf_front == -1 && cf_rear == -1){
    return;
  }
  int place;
  for(int i=0;i<7;i++){
    if(cfq.entry[i].cod == cod)
      place = i;
  }
  for(int i=place;i<7;i++){
    cfq.entry[i]=cfq.entry[i+1];
    cfq.p_s[i] = cfq.p_s[i+1];
    cfq.tag[i] = cfq.tag[i+1];
  }

  cfq.tag[7] = 'u';
  ins_init(&cfq.entry[7]);
  phy_reg_init(&cfq.p_s[7]);
}

void enqueue_iq(struct Stage* s){
  if(iq_front==0 && iq_rear==IQ_SIZE-1){
    return;
  }
  if(iq_front==-1)
    iq_front=0;
  iq_rear=iq_rear+1;
  iq.ins[iq_rear]=s->instruction_info;
}

void enqueue_lsq(struct Stage* s){
  if(lsq_front==0 && lsq_rear==LSQ_SIZE-1){
    return;
  }
  if(lsq_front==-1)
    lsq_front=0;
  lsq.ins[++lsq_rear]=s->instruction_info;
}
int enqueue_rob(struct Stage* s){
  if((front == 0 && rear ==  ROB_SIZE -1) || (front == rear+1)){
    return 0;
  }
  else{
    if(front==-1)
      front = 0;
    if(rear==ROB_SIZE-1)
      rear = 0;
    else
      rear = rear + 1;
    rob.entry[rear] = s->instruction_info;
    rob.tag[rear]='w';
    return 1;
  }
}

struct InstructionInfo get_ins_lsq(int i){  //GET INSTRUCITON FROM LSQ
  return lsq.ins[i];
}

struct InstructionInfo get_ins_from_lsq(){  //GET INSTRUCTION FROM LSQ AFTER CHECKING
  struct InstructionInfo ins; 
  bool load_go = true;
  for(int i=0;i<=LSQ_SIZE-1;i++){
    if(!strcmp(lsq.ins[i].operation,"STORE")){
      if(lsq.ins[i].src1.status && lsq.ins[i].src2.status && lsq.ins[i].target_address!=-1 && !lsq.ins[i].issued && (rob.entry[front].cod==lsq.ins[i].cod)){
        lsq.ins[i].issued = true;
        return get_ins_lsq(i);
      }
    }
    else{
      if(lsq.ins[i].src1.status && lsq.ins[i].src2.status && lsq.ins[i].target_address!=-1 && !lsq.ins[i].issued){
        for(int k=i-1;k>=0;k--){
          if(!strcmp(lsq.ins[k].operation,"STORE") && lsq.ins[k].target_address==-1)
            load_go=false;
        }
        if(load_go){
          lsq.ins[i].issued = true;
          return get_ins_lsq(i);
        }
        else{
          goto NEXT;
        }
      }
    }
    NEXT:
    ;
  }
  ins_init(&ins);
  return ins;
}

void dequeue_rob(){
  struct InstructionInfo ins;
  ins_init(&ins);
  if(front == -1){
    return;
  }
  else{
    rob.entry[front] = ins;
    rob.tag[front]='u';
    if(front == rear){
      front = -1;
      rear = -1;
    }
    else if(front == ROB_SIZE -1){
      front=0;
    }
    else{
      front = front +1;
    }
  }
}

void dequeue_lsq(struct InstructionInfo* ins){    //  DEQUEUE THE INSTRUCTION PASSED AS THE ARGUMENT
  int place;
  int last;
  for(int i=0;i<=LSQ_SIZE-1;i++){
    if(ins->cod == lsq.ins[i].cod){
      place =i;
      ins_init(&lsq.ins[i]);
    }
  }
  for(int i=place;i<=LSQ_SIZE-2;i++)
    lsq.ins[i]=lsq.ins[i+1];
  ins_init(&lsq.ins[LSQ_SIZE-1]);
  for(int i=LSQ_SIZE-1;i>=0;i--){
    if(!strcmp(lsq.ins[i].instruction.instruction_string," "))
      last = i;
  }
  lsq_rear=last-1;
}

void dequeue_iq(struct InstructionInfo* ins){     //  DEQUEUE THE INSTRUCTION PASSED AS THE ARGUMENT
  int place=-1;
  int last;
  for(int i=0;i<=IQ_SIZE-1;i++){
    if(ins->cod == iq.ins[i].cod){
      place = i;
      ins_init(&iq.ins[i]);
    }
  }
  if(place==-1)
    return;
  for(int i=place;i<=IQ_SIZE-2;i++)
    iq.ins[i]=iq.ins[i+1];
  ins_init(&iq.ins[IQ_SIZE-1]);
  for(int i=IQ_SIZE-1;i>=0;i--){
    if(!strcmp(iq.ins[i].instruction.instruction_string," "))
      last =i;
  }
  iq_rear=last-1;
}
bool all_done(){
  bool done = true;
  for(int i=0;i<LSQ_SIZE-1;i++){
    if(strcmp(lsq.ins[i].instruction.instruction_string," "))
      done = false;
  }

  for(int i=0;i<IQ_SIZE-1;i++){
    if(strcmp(iq.ins[i].instruction.instruction_string," "))
      done = false;
  }

  for(int i=0;i<ROB_SIZE-1;i++){
    if(strcmp(rob.entry[i].instruction.instruction_string," "))
      done = false;
  }
  }
  bool no_rob_slot(){
 if((front == 0 && rear ==   ROB_SIZE -1) || (front == rear+1))
  return true;
 return false;
}
void iq_init(){ //INITIALIZE ISSUE QUEUE
  for(int i=0;i<=IQ_SIZE-1;i++){
    ins_init(&iq.ins[i]);
  }
  iq_front=-1;
  iq_rear=-1;
}

void lsq_init(){  //INITIALIZE LSQ 
  for(int i=0;i<=LSQ_SIZE-1;i++){
    ins_init(&lsq.ins[i]);
  }
  lsq_front = -1;
  lsq_rear = -1;
}
void clear_rob(){   //CLEAR ROB
  struct InstructionInfo ins;
  ins_init(&ins);
  for(int i=0;i<=ROB_SIZE-1;i++){
    rob.entry[i]=ins;
    rob.tag[i]='u';
  }
  front = -1;
  rear = -1;
}

/*
 *  Execute Stage of APEX Pipeline implementation
 */
int execute(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[EX];
  if (!stage->busy && !stage->stalled) 
  {
    for(int i=0;i<=ROB_SIZE-1;i++){
        if(d4.instruction_info.cod == rob.entry[i].cod){    //UPDATE ROB DEST ENTRY AND CHANGE TAG IF IT INSTRUCTION HAS DESTINATION  
          rob.entry[i].dest = d4.instruction_info.dest;
          rob.tag[i]='c';
        }
      }
      for(int i=0;i<=ROB_SIZE-1;i++){
        if(m2.instruction_info.cod == rob.entry[i].cod){    //UPDATE ROB DEST ENTRY AND CHANGE TAG IF IT INSTRUCTION HAS DESTINATION  
          rob.entry[i].dest = m2.instruction_info.dest;
          rob.tag[i]='c';
        }
      }
      if(!in.stalled){
    if(stage_is_ready(&in)){
      in.instruction_info = get_int_from_iq();
      if(stage_will_write(&in)){
        dequeue_iq(&in.instruction_info);
        change_dest_to_invalid(&in);
        update_rob_tag(&in.instruction_info);
      }
    }
    else if(!strcmp(op,"JAL")){
      in.instruction_info.dest.value = in.instruction_info.PC + 4;
      in.instruction_info.target_address = in.instruction_info.src1.value + in.instruction_info.literal - 4;
      flush_due_to_branch(in.instruction_info.cod);
      if(stage_will_write(&d1)){
          dequeue_iq(&d1.instruction_info);
          update_rob_tag(&d1.instruction_info);
        }
        for(int i=0;i<=ROB_SIZE-1;i++){
        if(in.instruction_info.cod == rob.entry[i].cod && strcmp(in.instruction_info.operation,"LOAD") && strcmp(in.instruction_info.operation,"STORE")){   //UPDATE ROB DEST ENTRY AND CHANGE TAG IF IT INSTRUCTION HAS DESTINATION  
          rob.entry[i].dest = in.instruction_info.dest;
          rob.tag[i]='c';
        }
      }
    if (strcmp(stage->opcode, "STORE") == 0) 
    {
    stage->mem_address = stage->rs2_value + stage->imm;
    }

    if (strcmp(stage->opcode, "LOAD") == 0) 
   {
   	cpu->regs_valid[stage->rd]--;
    stage->mem_address = stage->imm+stage->rs1_value;
    }

  if (strcmp(stage->opcode, "JUMP") == 0) 
  { 
  	//strcpy(cpu->stage[DRF].opcode, "");
    //strcpy(cpu->stage[F].opcode, "");
  	
    stage->buffer = stage->rs1_value + stage->imm;
  	//printf("\nJUMP pc: %d\n", stage->buffer);
  }
  
  //bz
  if (strcmp(stage->opcode, "BZ") == 0)
  { 
    if(cpu->zero==1)
    {
      stage->mem_address = stage->pc + stage->imm;
      cpu->zero=0;
    } 
    else
     {
       stage->mem_address = 0;
     }
  }
  
  if (strcmp(stage->opcode, "BNZ") == 0) 
  { 
    if(!cpu->zero)
    {
      stage->mem_address = stage->pc + stage->imm;
    } 
    else 
    {
          stage->mem_address = 0;
      }
  }

    if (strcmp(stage->opcode, "MOVC") == 0) 
    {
      stage->buffer = stage->imm;
      cpu->buff[stage->rd]=stage->buffer;
      cpu->buff_valid[stage->rd]++;
    }
  
  if (strcmp(stage->opcode, "ADD") == 0) 
  {
   // printf(" RS1 %d  RS2 %d\n", stage->rs1_value ,stage->rs2_value);
    stage->buffer = stage->rs1_value + stage->rs2_value;
    cpu->buff[stage->rd]=stage->buffer;
    cpu->buff_valid[stage->rd]++;
  //  printf(" Add After exec %d\n",stage->buffer);
    if(stage->buffer==0)
       cpu->zero=1;
     else
       cpu->zero=0;   
    }
  
  if (strcmp(stage->opcode, "SUB") == 0) 
  {
    stage->buffer = stage->rs1_value - stage->rs2_value;
    cpu->buff[stage->rd]=stage->buffer;
    cpu->buff_valid[stage->rd]++;

    if(stage->buffer==0)
      cpu->zero=1;
    else
       cpu->zero=0; 
    }
  
  if (strcmp(stage->opcode, "AND") == 0) 
  {
    stage->buffer = stage->rs2_value & stage->rs1_value;
    cpu->buff[stage->rd]=stage->buffer;
    cpu->buff_valid[stage->rd]++;
  }
  
  if (strcmp(stage->opcode, "OR") == 0) 
  {
    stage->buffer = stage->rs2_value | stage->rs1_value;
    cpu->buff[stage->rd]=stage->buffer;
    cpu->buff_valid[stage->rd]++;
  }
  
  if (strcmp(stage->opcode, "XOR") == 0) 
  {
    stage->buffer = stage->rs2_value ^ stage->rs1_value;
    cpu->buff[stage->rd]=stage->buffer;
    cpu->buff_valid[stage->rd]++;
  }

  if (strcmp(stage->opcode, "MUL") == 0) 
  {
    if(stage->mul_flag==0)
    {
      cpu->stage[F].stalled=1;
      cpu->stage[DRF].stalled=1;
      cpu->stage[F].busy=1;
      cpu->stage[DRF].busy=1;
      stage->nop=1;
    }
    
    if(stage->mul_flag==1)
    {
      stage->buffer = stage->rs1_value * stage->rs2_value;
      cpu->stage[F].stalled=0;
      cpu->stage[DRF].stalled=0;
      cpu->stage[F].busy=0;
      cpu->stage[DRF].busy=0;
      stage->nop=0;
      cpu->buff[stage->rd]=stage->buffer;
      cpu->buff_valid[stage->rd]++;
    }
    stage->mul_flag=1;
    if(stage->buffer==0)
      cpu->zero=1;
    else
      cpu->zero=0;
    }

    if(strcmp(stage->opcode, "HALT") == 0)
     {
       stage->flush=1;
       cpu->stage[DRF].pc = 0;
       strcpy(cpu->stage[DRF].opcode, "");
       cpu->stage[DRF].stalled = 1;
       cpu->stage[F].stalled = 1;
       strcpy(cpu->stage[F].opcode, "");
       cpu->stage[F].pc = 0;
       cpu->ex_halt=1;
    }

    /* Copy data from Execute latch to Memory latch*/
    cpu->stage[MEM] = cpu->stage[EX];

    if(ENABLE_DEBUG_MESSAGES)
     {
      print_stage_content("Execute", stage);
     }
  }
  
  else
  {
   cpu->stage[MEM] = cpu->stage[EX];
  if(ENABLE_DEBUG_MESSAGES)
   {
      printf("Execute        : EMPTY\n");
    } 
  } 
  return 0;
}

/*
 *  Memory Stage of APEX Pipeline implementation
 */
int memory(APEX_CPU* cpu)
{
  if(!me.stalled){
    if(stage_is_ready(&me)){
      me.instruction_info = get_ins_from_lsq();
      if(!strcmp(me.instruction_info.operation,"STORE"))
        dequeue_rob();
      dequeue_lsq(&me.instruction_info);
      for(int i=0;i<=31;i++){
        if(rob.entry[i].cod == me.instruction_info.cod && me.instruction_info.cod!=-1)
          rob.tag[i] = 'e';
      }
    }
  CPU_Stage* stage = &cpu->stage[MEM];
  if (!stage->busy && !stage->stalled && stage->nop==0) 
  {

	if (strcmp(stage->opcode, "JUMP") == 0) 
 	{

	if(stage->buffer != 0) 
      {
        cpu->pc =stage->buffer;
         cpu->stage[DRF].pc = 0;
         strcpy(cpu->stage[DRF].opcode, "");
         strcpy(cpu->stage[EX].opcode, "");
         cpu->stage[EX].pc = 0;
        if((stage->buffer-4000) < 0) 
        {
          cpu->ins_completed = (cpu->ins_completed + ((stage->buffer-4000)/4))-1;
        }

        else
        {
          cpu->ins_completed = (cpu->ins_completed + ((stage->buffer-4000)/4)-cpu->ins_completed - 1);
        }

        if(cpu->ex_halt) 
        {
          cpu->ex_halt = 0;
          cpu->stage[F].stalled =0;
        }
      }
	}



    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0) 
    {
    cpu->data_memory[stage->mem_address] = stage->rs1_value;
    for(int i=0;i<=31;i++){
          if(rob.entry[i].cod == me.instruction_info.cod && me.instruction_info.cod!=-1)
            rob.tag[i] = 'e';
        }
        dequeue_lsq(&me.instruction_info);
        if(!strcmp(me.instruction_info.operation,"STORE"))
          dequeue_rob();
        if(stage_will_write(&me))
          mv++;
    }
  
    /* LOAD */
    if (strcmp(stage->opcode, "LOAD") == 0) 
    {

    	stage->buffer= cpu->data_memory[stage->mem_address];
       cpu->buff[stage->rd]=stage->buffer;
    	cpu->regs_valid[stage->rd]++;
    	cpu->buff_valid[stage->rd]++;

      
     // when store is in DRF
     // if(strcmp(cpu->stage[DRF].opcode, "STORE")==0)
      //{
      //cpu->buff_valid[stage->rd]++;
      //}
    }

    if (strcmp(stage->opcode, "BZ") == 0) 
    {
      if(stage->mem_address != 0) 
      {
        cpu->pc =stage->mem_address;
        if((strcmp(cpu->stage[EX].opcode, "ADD") == 0) || (strcmp(cpu->stage[EX].opcode, "SUB") == 0) || (strcmp(cpu->stage[EX].opcode, "MUL") == 0) || (strcmp(cpu->stage[EX].opcode, "AND") == 0) || (strcmp(cpu->stage[EX].opcode, "OR") == 0) || (strcmp(cpu->stage[EX].opcode, "XOR") == 0) || (strcmp(cpu->stage[EX].opcode, "MOVC") == 0) || (strcmp(cpu->stage[EX].opcode, "LOAD") == 0))
        {
          cpu->regs_valid[cpu->stage[EX].rd]++;
        }
         cpu->stage[DRF].pc = 0;
         strcpy(cpu->stage[DRF].opcode, "");
         strcpy(cpu->stage[EX].opcode, "");
         cpu->stage[EX].pc = 0;
        if(stage->imm < 0) 
        {
          cpu->ins_completed = (cpu->ins_completed + (stage->imm/4))-1;
        }

        else
        {
          cpu->ins_completed = (cpu->ins_completed - (stage->imm/4));
        }

        if(cpu->ex_halt) 
        {
          cpu->ex_halt = 0;
          cpu->stage[F].stalled =0;
        }
      }
    }
  
    if (strcmp(stage->opcode, "BNZ") == 0) 
    {
      if(stage->mem_address != 0) 
      {
        cpu->pc =stage->mem_address;

           if((strcmp(cpu->stage[EX].opcode, "ADD") == 0) || (strcmp(cpu->stage[EX].opcode, "SUB") == 0) || (strcmp(cpu->stage[EX].opcode, "MUL") == 0) || (strcmp(cpu->stage[EX].opcode, "AND") == 0) || (strcmp(cpu->stage[EX].opcode, "OR") == 0) || (strcmp(cpu->stage[EX].opcode, "XOR") == 0) || (strcmp(cpu->stage[EX].opcode, "MOVC") == 0) || (strcmp(cpu->stage[EX].opcode, "LOAD") == 0)) 
           {
              cpu->regs_valid[cpu->stage[EX].rd]++;
           }

         cpu->stage[DRF].pc = 0;
         strcpy(cpu->stage[DRF].opcode, "");
         strcpy(cpu->stage[EX].opcode, "");
         cpu->stage[EX].pc = 0;

        if(stage->imm < 0) 
        {
          cpu->ins_completed = (cpu->ins_completed + (stage->imm/4))-1;
        }

        else
        {
          cpu->ins_completed = (cpu->ins_completed - (stage->imm/4));
        }

        if(cpu->ex_halt) 
        {
          cpu->ex_halt = 0;
          cpu->stage[F].stalled =0;
        }
      }
    }

  if(strcmp(stage->opcode, "HALT") == 0) 
  {
      cpu->stage[EX].pc = 0;
      strcpy(cpu->stage[EX].opcode, "");
      cpu->stage[DRF].pc = 0;
      strcpy(cpu->stage[DRF].opcode, "");
      cpu->stage[EX].stalled = 1;
      cpu->stage[DRF].stalled = 1;
      strcpy(cpu->stage[F].opcode, "");
      cpu->stage[F].stalled = 1;
      cpu->stage[F].pc = 0;
      cpu->ex_halt=1;
    }

    /* Copy data from decode latch to execute latch*/
    cpu->stage[WB] = cpu->stage[MEM];

    if (ENABLE_DEBUG_MESSAGES) 
    {
      print_stage_content("Memory", stage);
    }
    dequeue_lsq(&me.instruction_info);
    for(int i=0;i<=31;i++){
          if(rob.entry[i].cod == me.instruction_info.cod && me.instruction_info.cod!=-1)
            rob.tag[i] = 'e';
        }
        if(!strcmp(me.instruction_info.operation,"STORE"))
          dequeue_rob();
        if(stage_will_write(&me))
          mv++;
  }
  
   else
   {
   cpu->stage[WB] = cpu->stage[MEM];
    if(ENABLE_DEBUG_MESSAGES)
    {
       printf("Memory         : EMPTY\n");
      } 
  }
  return 0;
}

/*
 *  Writeback Stage of APEX Pipeline implementation
 */
int writeback(APEX_CPU* cpu)
{
  /*if(instruction_will_write(&rob.entry[front])){
        if(rob.entry[front].dest.status && rob.tag[front] == 'c'){*/
  CPU_Stage* stage = &cpu->stage[WB];
  COMMIT:
      if(instruction_will_write(&rob.entry[front])){
        if(rob.entry[front].dest.status && rob.tag[front] == 'c'){
          commit++;
          if(!strcmp(committed_instructions_list[0].instruction.instruction_string," "))
            committed_instructions_list[0] = rob.entry[front];
          else if(!strcmp(committed_instructions_list[1].instruction.instruction_string," "))
            committed_instructions_list[1] = rob.entry[front];
          commit_to_arf();
          committed_instructions+=1;
          free_up_pr(&rob.entry[front]);  
          dequeue_rob();
          if(commit<2){
            goto COMMIT;
          }
        }
      }
    commit=0;
  if (!stage->busy && !stage->stalled && stage->nop==0 && (strcmp(stage->opcode, "")!=0)) 
  {
    /* Update register file */
    if (strcmp(stage->opcode, "MOVC") == 0) 
    {
      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd]++;
      cpu->stage[DRF].stalled=0;
      cpu->stage[F].stalled=0;  
    }
  
  if (strcmp(stage->opcode, "LOAD") == 0) 
  {
    if(!strcmp(in.instruction_info.operation,"LOAD")){
      if(bypass_condition(&in.instruction_info)){   //CONDITION TO BYPASS THE STORE IN LSQ AND GO FIRST IN MEM
        load_bypass(&in.instruction_info);
      }
      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd]++;
      cpu->stage[DRF].stalled=0;
      cpu->stage[F].stalled=0; 
      cpu->buff_valid[stage->rd]--;
      
    }
  
  
  if (strcmp(stage->opcode, "ADD") == 0) 
  {
      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd]++;
      cpu->stage[DRF].stalled=0;
      cpu->stage[F].stalled=0; 
      cpu->buff_valid[stage->rd]--;
  }
  
  if (strcmp(stage->opcode, "SUB") == 0) 
  {
      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd]++;
      cpu->stage[DRF].stalled=0;
      cpu->stage[F].stalled=0; 
      cpu->buff_valid[stage->rd]--;
  }
  
  if (strcmp(stage->opcode, "AND") == 0) 
  {
      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd]++;
      cpu->stage[DRF].stalled=0;
      cpu->stage[F].stalled=0; 
      cpu->buff_valid[stage->rd]--;
  }
  
  if (strcmp(stage->opcode, "OR") == 0) 
  {
      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd]++;
      cpu->stage[DRF].stalled=0;
      cpu->stage[F].stalled=0; 
      cpu->buff_valid[stage->rd]--;
  }
  
  if (strcmp(stage->opcode, "XOR") == 0) 
  {
      cpu->regs[stage->rd] = stage->buffer;
    cpu->regs_valid[stage->rd]++;
    cpu->stage[DRF].stalled=0;
    cpu->stage[F].stalled=0; 
    cpu->buff_valid[stage->rd]--;
    }
  
  if (strcmp(stage->opcode, "MUL") == 0) 
  {
      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd]++;
      cpu->stage[DRF].stalled=0;
      cpu->stage[F].stalled=0; 
      cpu->buff_valid[stage->rd]--;
  }

    if(strcmp(stage->opcode, "HALT") == 0) 
    {
        cpu->ins_completed = cpu->code_memory_size - 1;
        cpu->stage[EX].pc = 0;
        strcpy(cpu->stage[EX].opcode, "");
        cpu->stage[DRF].pc = 0;
        strcpy(cpu->stage[DRF].opcode, "");
        cpu->stage[EX].stalled = 1;
        cpu->stage[DRF].stalled = 1;
        cpu->stage[F].stalled = 1;
        strcpy(cpu->stage[F].opcode, "");
        cpu->stage[F].pc = 0;
        cpu->stage[MEM].pc = 0;
        strcpy(cpu->stage[MEM].opcode, "");
        cpu->stage[MEM].stalled = 1;
        cpu->ex_halt=1;
    }
  
    cpu->ins_completed++;

    if (ENABLE_DEBUG_MESSAGES) 
    {
      print_stage_content("Writeback", stage);
    }
  }
  
   else
  {
   
  if(ENABLE_DEBUG_MESSAGES)
   {
      printf("Writeback      : EMPTY\n");
    } 
  }
  return 0;
}

display_lsq();
display_isq();
display_rob();

void display(APEX_CPU* cpu)                      // Display Register values
  {
    for(int i=0; i<16; i++)
    {
    printf("\nR%d = %d",i,cpu->regs[i]);
    }
  }
  
void valid(APEX_CPU* cpu)
  {
    for(int i=0;i<16;i++)
    {
      cpu->regs_valid[i]=2;
    }
  }
    

void validAll(APEX_CPU* cpu)
{
  for(int i=0;i<16;i++)
  {
    cpu->regs_valid[i]=1;
  }
}

void dispRegValue(APEX_CPU* cpu)
{
  for(int i=0;i<16;i++)
  {
    printf("\n\tR%d || Value = %d",i,cpu->regs[i]);
  }
}

/*void dispRegValid(APEX_CPU* cpu)
{
  for(int i=0;i<16;i++)
  {
    printf("\n\tR%d || Validity = %d",i,cpu->regs_valid[i]);
  }
}*/

void dispMem(APEX_CPU* cpu)
{
  for(int i=0;i<100;i++)
  {
    printf("\n\tMEM_Value[%d] || Value=%d\n",i,cpu->data_memory[i]);
  }
}
void display_lsq(){
  printf("\n    Load-Store Queue\n");
  printf("\n------------------------");
  for(int i=LSQ_SIZE-1;i>=0;i--){
    if(strcmp(lsq.ins[i].instruction.instruction_string," ")){
      printf("\n%2d | ",i);
      print_instruction_after_rename(&lsq.ins[i]);
      printf(" |");
    }
  }
  printf("\n------------------------\n");
}
void display_rob(){
  printf("\n\t\t    ROB\t\n");
  printf("\n-----------------------------------------------------------");
  printf("\n| No | | Instruction                 | PhyReg | Value | Tag |");
  printf("\n-----------------------------------------------------------");
  for(int i=0;i<=ROB_SIZE-1;i++){
        printf("\n| %2d | ",i);
      rob_entry_print(&rob.entry[i]);
      printf(" | %6s |",rob.entry[i].dest.name);
      if(rob.tag[i] == 'c')
        printf(" %5d | (%c) |",rob.entry[i].dest.value,rob.tag[i]);
      else
        printf("       | (%c) |",rob.tag[i]);
      if(i==front)
        printf("\t<-HEAD");
      if(i==rear)
        printf("\t<-TAIL");
  }
  printf("\n-----------------------------------------------------------");
}
void rob_entry_print(struct InstructionInfo* ins){
  printf("|");
  if(strcmp(ins->instruction.instruction_string," ")){
    print_instruction_after_rename(ins);
  }
  else
    printf("%26s",ins->instruction.instruction_string);
}

/*
 *  APEX CPU simulation loop
 */
int APEX_cpu_run(APEX_CPU* cpu)
{
  //valid(cpu);
  while (1) 
  {
    /* All the instructions committed, so exit */
   // printf("INS Complete  %d \n",cpu->ins_completed );
   // printf("Mem size %d\n",cpu->code_memory_size );
    if (cpu->ins_completed == cpu->code_memory_size || cpu->clock==cpu->no_cycles) 
    {
     // printf("Inside here  \n");
     // printf("\n%d==%d || %d==%d\n",cpu->ins_completed, cpu->code_memory_size, cpu->clock, cpu->no_cycles);
      printf("(apex) >> Simulation Complete");
      break;
    }



    if(ENABLE_DEBUG_MESSAGES)
    {
      printf("--------------------------------\n");
      printf("Clock Cycle #: %d\n", cpu->clock);
      printf("--------------------------------\n");
    }
  
  // printf("\n\n\n============================REGISTER VALID=========================");
  // dispRegValid(cpu);
  
    writeback(cpu);
    memory(cpu);
    execute(cpu);
    decode(cpu);
    fetch(cpu);
    cpu->clock++;
}
  printf("\n");
  printf("=====REGISTER VALUE============\n");
  for(int i=0;i<16;i++)
  {printf("\n");
  printf(" | Register[%d] | Value=%d | status=%s | \n",i,cpu->regs[i],(cpu->regs_valid[i])?"Valid" : "Invalid");
      
  }
printf("=======DATA MEMORY===========\n");

  for(int i=0;i<99;i++)
  {
  printf(" | MEM[%d] | Value=%d | \n",i,cpu->data_memory[i]);
  }
  return 0;
  }