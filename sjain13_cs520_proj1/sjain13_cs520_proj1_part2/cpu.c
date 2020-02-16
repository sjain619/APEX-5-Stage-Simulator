//Part 2
/*
 *  cpu.c
 *  Contains APEX cpu pipeline implementation
 *
 *  Author :
 *  Gaurav Kothari (gkothar1@binghamton.edu)
 *  State University of New York, Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"

/* Set this flag to 1 to enable debug messages */
#define ENABLE_DEBUG_MESSAGES 1
int mul_count = 0;
int halt=0;
int hck = 0;

/*
 * This function creates and initializes APEX cpu.
 *
 * Note : You are free to edit this function according to your
 * 				implementation
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
  	memset(cpu->regs, 0, sizeof(int) * 32);
  	memset(cpu->regs_valid, 1, sizeof(int) * 32);
	memset(cpu->ex, 0, sizeof(int) * 32);
  	memset(cpu->ex_valid, 0, sizeof(int) * 32);
	memset(cpu->stage, 0, sizeof(CPU_Stage) * NUM_STAGES);
  	memset(cpu->data_memory, 0, sizeof(int) * 4000);

  	/* Parse input file and create code memory */
  	cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);

  	if (!cpu->code_memory)
	{
    		free(cpu);
    		return NULL;
  	}

  	if (ENABLE_DEBUG_MESSAGES)
	{
    		fprintf(stderr,"APEX_CPU : Initialized APEX CPU, loaded %d instructions\n",cpu->code_memory_size);
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

  	/* Make all stages busy except Fetch stage, initially to start the pipeline */
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
 * 				implementation
 */
void
APEX_cpu_stop(APEX_CPU* cpu)
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
int
get_code_index(int pc)
{
  	return (pc - 4000) / 4;
}

static void
print_instruction(CPU_Stage* stage)
{
  	if (strcmp(stage->opcode, "STORE") == 0)
	{
        printf("%s,R%d,R%d,#%d ", stage->opcode, stage->rs1, stage->rs2, stage->imm);
  	}
  	if (strcmp(stage->opcode, "LOAD") == 0)
	{
        printf("%s,R%d,R%d,#%d ", stage->opcode, stage->rd, stage->rs1, stage->imm);
  	}
	if(strcmp(stage->opcode, "ADD")== 0)
	{
   		printf("%s,R%d,R%d,R%d",stage->opcode,stage->rd,stage->rs1,stage->rs2);
  	}
	if(strcmp(stage->opcode, "MUL")== 0)
	{
   		printf("%s,R%d,R%d,R%d",stage->opcode,stage->rd,stage->rs1,stage->rs2);
  	}
	if(strcmp(stage->opcode, "SUB")== 0)
	{
   		printf("%s,R%d,R%d,R%d",stage->opcode,stage->rd,stage->rs1,stage->rs2);
  	}
	if(strcmp(stage->opcode, "XOR")== 0)
	{
   		printf("%s,R%d,R%d,R%d",stage->opcode,stage->rd,stage->rs1,stage->rs2);
  	}
	if(strcmp(stage->opcode, "OR")== 0)
	{
   		printf("%s,R%d,R%d,R%d",stage->opcode,stage->rd,stage->rs1,stage->rs2);
  	}
	if(strcmp(stage->opcode, "AND")== 0)
	{
   		printf("%s,R%d,R%d,R%d",stage->opcode,stage->rd,stage->rs1,stage->rs2);
  	}
	if (strcmp(stage->opcode, "MOVC") == 0)
	{
        printf("%s,R%d,#%d ", stage->opcode, stage->rd, stage->imm);
  	}
    if (strcmp(stage->opcode, "BZ") == 0)
	{
        printf("%s,#%d ", stage->opcode, stage->imm);
  	}
    	if (strcmp(stage->opcode, "BNZ") == 0)
	{
        printf("%s,#%d ", stage->opcode, stage->imm);
  	}
	if (strcmp(stage->opcode, "HALT") == 0)
	{
		printf("HALT");
	}
	if (strcmp(stage->opcode, "JUMP") == 0)
	{
    		printf("%s,R%d,#%d", stage->opcode,stage->rs1,stage->imm);
  	}
}

/* Debug function which dumps the cpu stage
 * content
 *
 * Note : You are not supposed to edit this function
 *
 */
static void
print_stage_content(char* name, CPU_Stage* stage)
{
  	printf("Instruction at %-15s Stage: (I%d : %d) ", name,(stage->pc - 4000)/4,stage->pc);
  	print_instruction(stage);
  	printf("\n");
}

static void
print_stage_contents(char* name)
{
  printf("%-15s: EMPTY ", name);
  printf(" ");
  printf("\n");
}

/*
 *  Fetch Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
fetch(APEX_CPU* cpu)
{
  	CPU_Stage* stage = &cpu->stage[F];
  	if (!stage->busy && !stage->stalled && halt!=1)
	{
    		/* Store current PC in fetch latch */
    		stage->pc = cpu->pc;
    		/* Index into code memory using this pc and copy all instruction fields into fetch latch */
    		APEX_Instruction* current_ins = &cpu->code_memory[get_code_index(cpu->pc)];
    		strcpy(stage->opcode, current_ins->opcode);
    		stage->rd = current_ins->rd;
    		stage->rs1 = current_ins->rs1;
    		stage->rs2 = current_ins->rs2;
    		stage->imm = current_ins->imm;
            if(cpu->stage[DRF].stalled==1)
            {
                //printf("\nDecode is stalled\n");
    			if (ENABLE_DEBUG_MESSAGES)
                {
     				print_stage_content("Fetch", stage);
                }
			return 0;
		}

            cpu->pc += 4;

		/* Update PC for next instruction */

		/* Copy data from fetch latch to decode latch*/
    		cpu->stage[DRF] = cpu->stage[F];
            if (ENABLE_DEBUG_MESSAGES)
            {
                print_stage_content("Fetch", stage);
            }
  	} else {
		if(ENABLE_DEBUG_MESSAGES) {
			print_stage_content("Fetch", stage);
		}
	}
	return 0;
}

/*
 *  Decode Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
decode(APEX_CPU* cpu)
{
  	CPU_Stage* stage = &cpu->stage[DRF];
	//printf("\nDecode opcode: %s\n",stage->opcode);
	/*if((cpu->regs_valid[stage->rs1]==1) || (cpu->regs_valid[stage->rs2]==1))
	{
		stage->stalled=1;
	}
	else
	stage->stalled=0;*/
  	if(cpu->stage[EX].busy==1 && strcmp(cpu->stage[EX].opcode, "MUL") == 0)
	{
  		stage->stalled=1;
  		if (ENABLE_DEBUG_MESSAGES)
		{
  			print_stage_content("Decode", stage);
  		}
  		return 0;
  	}
	if(mul_count == 0)
		stage->stalled = 0;
	/*if(cpu->regs_valid[stage->rd]==0)
	{
		cpu->stage[F].stalled=1;
  		cpu->stage[DRF].stalled=1;
	}
	else
	{
		cpu->stage[F].stalled=0;
  		cpu->stage[DRF].stalled=0;
	}*/
	if(stage->stalled==1)
	{
     		stage->stalled=0;
   	}
  	if (!stage->busy && !stage->stalled)
	{
		/*if(strcmp(stage->opcode, "HALT")==0){halt=1;
			strcpy(cpu->stage[F].opcode, "");
		}*/
		/* Read data from register file for store */
		if(strcmp(stage->opcode, "BZ")==0 || strcmp(stage->opcode, "BNZ")==0)
        {
            if(strcmp(cpu->stage[MEM].opcode, "ADD")==0 || strcmp(cpu->stage[WB].opcode, "ADD")==0 || strcmp(stage[MEM].opcode, "SUB")==0 || strcmp(stage[WB].opcode,"SUB")==0 || strcmp(stage[MEM].opcode,"MUL")==0 || strcmp(stage[WB].opcode,"MUL")==0)
            {
                cpu->stage[DRF].stalled=1;
                //cpu->stage[F].stalled=1;
            } else {
                cpu->stage[DRF].stalled=0;
                cpu->stage[F].stalled=0;
            }
        }
        if (strcmp(stage->opcode, "STORE") == 0)
	{
            	if(cpu->regs_valid[stage->rs1] == 1 || cpu->regs_valid[stage->rs2] == 1)
		{
			stage->stalled=1;
            	}
		else if((cpu->ex_valid[stage->rs1])&&(cpu->regs_valid[stage->rs2]))
                { 
                	stage->rs1_value=cpu->ex[stage->rs1];
    			stage->rs2_value=cpu->regs[stage->rs2];
		}
                else if((cpu->ex_valid[stage->rs1])&&(cpu->ex_valid[stage->rs2]))
                { 
                        stage->rs1_value=cpu->ex[stage->rs1];
    			stage->rs2_value=cpu->ex[stage->rs2];
				
                }

                else if((cpu->regs_valid[stage->rs1])&&(cpu->ex_valid[stage->rs2]))
                { 
                        stage->rs1_value=cpu->regs[stage->rs1];
    			stage->rs2_value=cpu->ex[stage->rs2];
				
                }

      		else
            	{
                	stage->rs1_value=cpu->regs[stage->rs1];
      			stage->rs2_value=cpu->regs[stage->rs2];
      			stage->stalled=0;
      			//stage->stage_stalled=0;
      		}
        }

		/* Read data from register file for load */
     		if (strcmp(stage->opcode, "LOAD") == 0)
            {
                
     			if(cpu->regs_valid[stage->rs1] == 1)
                {
      				stage->stalled=1;
      				//stage->stage_stalled=1;
    			}
    			else
                {
      				stage->rs1_value=cpu->regs[stage->rs1];
      				stage->stalled=0;
      				//stage->stage_stalled=0;
				cpu->regs_valid[stage->rd] = 1;
    			}
            }

   	 	/* No Register file read needed for MOVC*/
    		if (strcmp(stage->opcode, "MOVC") == 0)
		{
			cpu->regs_valid[stage->rd]=1;
		}

  		/* Read data from register file for Add */
    		if (strcmp(stage->opcode, "ADD") == 0)
            {
                //cpu->regs_valid[stage->rd]=1;
                if(cpu->regs_valid[stage->rs1] == 1 || cpu->regs_valid[stage->rs2] == 1)
                {
      				stage->stalled=1;
      				//stage->stage_stalled=1;
    			}
		else if((cpu->ex_valid[stage->rs1])&&(cpu->regs_valid[stage->rs2]))
                	{ 
                		stage->rs1_value=cpu->ex[stage->rs1];
    				stage->rs2_value=cpu->regs[stage->rs2];
				cpu->regs_valid[stage->rd]=0;
                	}
                	else if((cpu->ex_valid[stage->rs1])&&(cpu->ex_valid[stage->rs2]))
                	{ 
                        	stage->rs1_value=cpu->ex[stage->rs1];
    				stage->rs2_value=cpu->ex[stage->rs2];
				cpu->regs_valid[stage->rd]=0;
                	}

                	else if((cpu->regs_valid[stage->rs1])&&(cpu->ex_valid[stage->rs2]))
                	{ 
                        	stage->rs1_value=cpu->regs[stage->rs1];
    				stage->rs2_value=cpu->ex[stage->rs2];
				cpu->regs_valid[stage->rd]=0;
                	}
    			else
                {
      				stage->rs1_value=cpu->regs[stage->rs1];
      				stage->rs2_value=cpu->regs[stage->rs2];
        			stage->stalled=0;
        			cpu->regs_valid[stage->rd] =1;
				//stage->stage_stalled=0;
    			}
            }

        /* Read data from register file for Sub */
    		if (strcmp(stage->opcode, "SUB") == 0)
            {
                //cpu->regs_valid[stage->rd]=1;
                if(cpu->regs_valid[stage->rs1] == 1 || cpu->regs_valid[stage->rs2] == 1)
                {
      				stage->stalled=1;
      				//stage->stage_stalled=1;
    			}
		else if((cpu->ex_valid[stage->rs1])&&(cpu->regs_valid[stage->rs2]))
                	{ 
                		stage->rs1_value=cpu->ex[stage->rs1];
    				stage->rs2_value=cpu->regs[stage->rs2];
				cpu->regs_valid[stage->rd]=0;
                	}
                	else if((cpu->ex_valid[stage->rs1])&&(cpu->ex_valid[stage->rs2]))
                	{ 
                        	stage->rs1_value=cpu->ex[stage->rs1];
    				stage->rs2_value=cpu->ex[stage->rs2];
				cpu->regs_valid[stage->rd]=0;
                	}

                	else if((cpu->regs_valid[stage->rs1])&&(cpu->ex_valid[stage->rs2]))
                	{ 
                        	stage->rs1_value=cpu->regs[stage->rs1];
    				stage->rs2_value=cpu->ex[stage->rs2];
				cpu->regs_valid[stage->rd]=0;
                	}
    			else
                {
      				stage->rs1_value=cpu->regs[stage->rs1];
      				stage->rs2_value=cpu->regs[stage->rs2];
        			stage->stalled=0;
        			cpu->regs_valid[stage->rd] = 1;
				//stage->stage_stalled=0;
    			}
            }

            /* Read data from register file for Jump */
            if (strcmp(stage->opcode, "JUMP") == 0)
            {
       			if(cpu->regs_valid[stage->rs1] == 1)
                {
                    stage->stalled=1;
      			//stage->stage_stalled=1;
                }
                else
                {
                    stage->rs1_value= cpu->regs[stage->rs1];
                    stage->stalled=0;
		    //cpu->regs_valid[stage->rd] = 1;
                    //stage->stage_stalled=0;
                }
            }

    		/* Read data from register file for Xor */
    		if (strcmp(stage->opcode, "XOR") == 0)
            {
                //cpu->regs_valid[stage->rd]=1;
                if(cpu->regs_valid[stage->rs1] == 1 || cpu->regs_valid[stage->rs2] == 1)
                {
      				stage->stalled=1;
      				//stage->stage_stalled=1;
    			}
		else if((cpu->ex_valid[stage->rs1])&&(cpu->regs_valid[stage->rs2]))
                	{ 
                		stage->rs1_value=cpu->ex[stage->rs1];
    				stage->rs2_value=cpu->regs[stage->rs2];
				cpu->regs_valid[stage->rd]=0;
                	}
                	else if((cpu->ex_valid[stage->rs1])&&(cpu->ex_valid[stage->rs2]))
                	{ 
                        	stage->rs1_value=cpu->ex[stage->rs1];
    				stage->rs2_value=cpu->ex[stage->rs2];
				cpu->regs_valid[stage->rd]=0;
                	}

                	else if((cpu->regs_valid[stage->rs1])&&(cpu->ex_valid[stage->rs2]))
                	{ 
                        	stage->rs1_value=cpu->regs[stage->rs1];
    				stage->rs2_value=cpu->ex[stage->rs2];
				cpu->regs_valid[stage->rd]=0;
                	}
    			else
                {
      				stage->rs1_value=cpu->regs[stage->rs1];
      				stage->rs2_value=cpu->regs[stage->rs2];
        			stage->stalled=0;
        			cpu->regs_valid[stage->rd] = 1;
				//stage->stage_stalled=0;
                }
            }

            /* Read data from register file for OR */
    		if (strcmp(stage->opcode, "OR") == 0)
            {
                //cpu->regs_valid[stage->rd]=1;
                if(cpu->regs_valid[stage->rs1] == 1 || cpu->regs_valid[stage->rs2] == 1)
                {
      				stage->stalled=1;
      				//stage->stage_stalled=1;
    			}
		else if((cpu->ex_valid[stage->rs1])&&(cpu->regs_valid[stage->rs2]))
                	{ 
                		stage->rs1_value=cpu->ex[stage->rs1];
    				stage->rs2_value=cpu->regs[stage->rs2];
				cpu->regs_valid[stage->rd]=0;
                	}
                	else if((cpu->ex_valid[stage->rs1])&&(cpu->ex_valid[stage->rs2]))
                	{ 
                        	stage->rs1_value=cpu->ex[stage->rs1];
    				stage->rs2_value=cpu->ex[stage->rs2];
				cpu->regs_valid[stage->rd]=0;
                	}

                	else if((cpu->regs_valid[stage->rs1])&&(cpu->ex_valid[stage->rs2]))
                	{ 
                        	stage->rs1_value=cpu->regs[stage->rs1];
    				stage->rs2_value=cpu->ex[stage->rs2];
				cpu->regs_valid[stage->rd]=0;
                	}
    			else
                {
      				stage->rs1_value=cpu->regs[stage->rs1];
      				stage->rs2_value=cpu->regs[stage->rs2];
        			stage->stalled=0;
        			cpu->regs_valid[stage->rd] = 1;
				//stage->stage_stalled=0;
    			}
            }

            /* Read data from register file for AND */
    		if (strcmp(stage->opcode, "AND") == 0)
            {
                //printf("src1=%d:%d, src2:%d:%d\n",stage->rs1, cpu->regs_valid[stage->rs1], stage->rs2, cpu->regs_valid[stage->rs2]);
                if(cpu->regs_valid[stage->rs1] == 1 || cpu->regs_valid[stage->rs2] == 1)
                {
      				stage->stalled=1;
      				//stage->stage_stalled=1;
    			}
		else if((cpu->ex_valid[stage->rs1])&&(cpu->regs_valid[stage->rs2]))
                	{ 
                		stage->rs1_value=cpu->ex[stage->rs1];
    				stage->rs2_value=cpu->regs[stage->rs2];
				cpu->regs_valid[stage->rd]=0;
                	}
                	else if((cpu->ex_valid[stage->rs1])&&(cpu->ex_valid[stage->rs2]))
                	{ 
                        	stage->rs1_value=cpu->ex[stage->rs1];
    				stage->rs2_value=cpu->ex[stage->rs2];
				cpu->regs_valid[stage->rd]=0;
                	}

                	else if((cpu->regs_valid[stage->rs1])&&(cpu->ex_valid[stage->rs2]))
                	{ 
                        	stage->rs1_value=cpu->regs[stage->rs1];
    				stage->rs2_value=cpu->ex[stage->rs2];
				cpu->regs_valid[stage->rd]=0;
                	}
    			else
                {
      				stage->rs1_value=cpu->regs[stage->rs1];
      				stage->rs2_value=cpu->regs[stage->rs2];
        			stage->stalled=0;
        			cpu->regs_valid[stage->rd] = 1;
				//stage->stage_stalled=0;
    			}
            }

    		if (strcmp(stage->opcode, "MUL") == 0)
            {
                //cpu->regs_valid[stage->rd]=1;
    			if(cpu->regs_valid[stage->rs1] == 1 || cpu->regs_valid[stage->rs2] == 1)
                {
    				/*stage->rs1_value=cpu->regs[stage->rs1];
    				stage->rs2_value=cpu->regs[stage->rs2];
    				cpu->regs_valid[stage->rd]=0;*/
                    stage->stalled=1;
    			}
		else if((cpu->ex_valid[stage->rs1])&&(cpu->regs_valid[stage->rs2]))
                	{ 
                		stage->rs1_value=cpu->ex[stage->rs1];
    				stage->rs2_value=cpu->regs[stage->rs2];
				cpu->regs_valid[stage->rd]=0;
                	}
                	else if((cpu->ex_valid[stage->rs1])&&(cpu->ex_valid[stage->rs2]))
                	{ 
                        	stage->rs1_value=cpu->ex[stage->rs1];
    				stage->rs2_value=cpu->ex[stage->rs2];
				cpu->regs_valid[stage->rd]=0;
                	}

                	else if((cpu->regs_valid[stage->rs1])&&(cpu->ex_valid[stage->rs2]))
                	{ 
                        	stage->rs1_value=cpu->regs[stage->rs1];
    				stage->rs2_value=cpu->ex[stage->rs2];
				cpu->regs_valid[stage->rd]=0;
                	}
                else
                {
      				stage->rs1_value=cpu->regs[stage->rs1];
      				stage->rs2_value=cpu->regs[stage->rs2];
        			stage->stalled=0;
        			cpu->regs_valid[stage->rd] = 1;
				//stage->stage_stalled=0;
    			}
            }


    		/* Copy data from decode latch to execute latch*/
    		cpu->stage[EX] = cpu->stage[DRF];
            if (ENABLE_DEBUG_MESSAGES)
            {
      			print_stage_content("Decode/RF", stage);
    		}

	}
	else
    {
        cpu->stage[EX] = cpu->stage[DRF];
        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_contents("Decode");
        }
    }
    return 0;
}

/*
 *  Execute Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
execute(APEX_CPU* cpu)
{
  	CPU_Stage* stage = &cpu->stage[EX];
	if(stage->busy && mul_count==1)
	stage->busy = 0;
	if(strcmp(cpu->stage[EX].opcode, "HALT") == 0 || strcmp(cpu->stage[MEM].opcode, "HALT") ==0 || strcmp(cpu->stage[WB].opcode, "HALT") == 0)
	{
		//stage->stalled=1;
		strcpy(cpu->stage[DRF].opcode,"");
		strcpy(cpu->stage[F].opcode, "");
		halt = 1;
		cpu->stage[MEM] = cpu->stage[EX];
		if (ENABLE_DEBUG_MESSAGES)
		{
  			print_stage_content("Decode", stage);
  		}
		return 0;
	}
	else if(hck == 1)   //Halt Check
	{
		cpu->stage[EX]=cpu->stage[DRF];
		if (ENABLE_DEBUG_MESSAGES)
		{
  			print_stage_content("Execute", stage);
  		}
	}

  	if (!stage->busy && !stage->stalled)
	{
		/* Store */
        if (strcmp(stage->opcode, "STORE") == 0)
		{
            stage->mem_address=(stage->rs2_value)+(stage->imm);
        }

        /* Load */
        if (strcmp(stage->opcode, "LOAD") == 0)
		{
            stage->mem_address=(stage->rs1_value)+(stage->imm);
			cpu->regs_valid[stage->rd] = 1;//#new
        }

        /* MOVC */
        if (strcmp(stage->opcode, "MOVC") == 0)
		{
            stage->buffer=0+(stage->imm);
		cpu->ex[stage->rd]=stage->buffer;
                 cpu->ex_valid[stage->rd]=1;

        }

		/* JUMP */
		if (strcmp(stage->opcode, "JUMP") == 0)
		{
            printf("cpu PC %d\n",cpu -> pc);
            cpu->pc = stage->rs1_value + stage->imm;
        }

        /* ADD */
        if (strcmp(stage->opcode, "ADD") == 0)
		{
            stage->buffer=(stage->rs1_value)+(stage->rs2_value);
		cpu->ex[stage->rd]=stage->buffer;
                        cpu->ex_valid[stage->rd]=1;
			//cpu->regs_valid[stage->rd]=0;
			cpu->regs_valid[stage->rd] = 1;//#new
        }

        /* SUB */
        if (strcmp(stage->opcode, "SUB") == 0)
		{
            stage->buffer=(stage->rs1_value)-(stage->rs2_value);
		cpu->ex[stage->rd]=stage->buffer;
                        cpu->ex_valid[stage->rd]=1;
			cpu->regs_valid[stage->rd]=1;
        }

        /* XOR */
        if (strcmp(stage->opcode, "XOR") == 0)
		{
            stage->buffer=(stage->rs1_value)^(stage->rs2_value);
		cpu->ex[stage->rd]=stage->buffer;
                        cpu->ex_valid[stage->rd]=1;
			cpu->regs_valid[stage->rd]=1;
        }

        /* OR */
        if (strcmp(stage->opcode, "OR") == 0)
		{
            stage->buffer=(stage->rs1_value)|(stage->rs2_value);
		cpu->ex[stage->rd]=stage->buffer;
                        cpu->ex_valid[stage->rd]=1;
			cpu->regs_valid[stage->rd]=1;
        }

        /* AND */
        if (strcmp(stage->opcode, "AND") == 0)
		{
            stage->buffer=(stage->rs1_value) & (stage->rs2_value);
		cpu->ex[stage->rd]=stage->buffer;
                        cpu->ex_valid[stage->rd]=1;
			cpu->regs_valid[stage->rd]=1;
        }

        /* MUL */
        if (strcmp(stage->opcode, "MUL") == 0)
		{
            stage->buffer=(stage->rs1_value) * (stage->rs2_value);
		cpu->ex[stage->rd]=stage->buffer;
                        cpu->ex_valid[stage->rd]=1;
			cpu->regs_valid[stage->rd]=1;
			if(mul_count == 0)
			{
                stage->busy=1;
				mul_count++;
			}
			else
			mul_count = 0;
        }
        /*BZ*/
        if (strcmp(stage->opcode, "BZ") == 0)
	{
		/*if(stage[WB].opcode == "ADD" || stage[WB].opcode == "SUB" || stage[WB].opcode == "MUL")
		{
			cpu->stage[F].stalled=1;
			cpu->stage[DRF].stalled=1;
			cpu->stage[EX].stalled=1;
		}
		else
		{*/
			if(cpu->zflag==1)
            		{
						stage->buffer=(stage->pc)+(stage->imm);
                		//cpu->pc=(stage->pc)+(stage->imm);
				//cpu->stage[F].stalled=0;
			//cpu->stage[DRF].stalled=0;

            		}
		}
	
            /*else
            printf("\nBZ does not satisfy the conditions\n");
		}*/

        /*BNZ*/
        if (strcmp(stage->opcode, "BNZ") == 0)
	{
		/*if(stage[WB].opcode == "ADD" || stage[WB].opcode == "SUB" || stage[WB].opcode == "MUL")
		{
			cpu->stage[F].stalled=1;
			cpu->stage[DRF].stalled=1;
			cpu->stage[EX].stalled=1;
		}
		else
		{*/
			if(cpu->zflag==0)
            		{
                		stage->buffer=(stage->pc)+(stage->imm);
						//cpu->pc=(stage->pc)+(stage->imm);
				//cpu->stage[F].stalled=0;
			//cpu->stage[DRF].stalled=0;

            		}
		
	}
           /*else
            printf("\nBNZ does not satisfy the conditions\n");
		}*/

        /* Copy data from Execute latch to Memory latch*/
        cpu->stage[MEM] = cpu->stage[EX];
		//cpu->stage[F].stalled=0;
        //cpu->stage[DRF].stalled=1;
		if (ENABLE_DEBUG_MESSAGES)
		{
            print_stage_content("Execute", stage);
        }
  	}
	else
    {
        cpu->stage[MEM] = cpu->stage[EX];
        if (ENABLE_DEBUG_MESSAGES)
        {
      		print_stage_contents("Execute");
        }
	}
 	return 0;
}

/*
 *  Memory Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int memory(APEX_CPU* cpu)
{
  	CPU_Stage* stage = &cpu->stage[MEM];
  	if (!stage->busy && !stage->stalled)
	{
        /* Store */
        if (strcmp(stage->opcode, "STORE") == 0)
		{
            cpu->data_memory[stage->mem_address]=stage->rs1_value;
        }

        /* Load */
        if (strcmp(stage->opcode, "LOAD") == 0)
		{
            stage->mem_address=cpu->data_memory[stage->mem_address];
            cpu->regs_valid[stage->rd]=1;
        }

        /* MOVC */
        if (strcmp(stage->opcode, "MOVC") == 0)
		{
        }

        /* ADD */
        if (strcmp(stage->opcode, "ADD") == 0)
		{
			cpu->regs_valid[stage->rd] = 1;//#new
        }

        /* SUB */
        if (strcmp(stage->opcode, "SUB") == 0)
		{
			cpu->regs_valid[stage->rd] = 1;//#new
        }

        /* XOR */
        if (strcmp(stage->opcode, "XOR") == 0)
		{
			cpu->regs_valid[stage->rd] = 1;//#new
        }

        /* OR */
        if (strcmp(stage->opcode, "OR") == 0)
		{
			cpu->regs_valid[stage->rd] = 1;//#new
        }

        /* AND */
        if (strcmp(stage->opcode, "AND") == 0)
		{
			cpu->regs_valid[stage->rd] = 1;//#new
        }

        /* MUL */
        if (strcmp(stage->opcode, "MUL") == 0)
		{
			cpu->regs_valid[stage->rd] = 1;//#new
        }

		/* HALT */
		if (strcmp(stage->opcode, "HALT") == 0)
		{
		}
		
		if(strcmp(stage->opcode,"BZ")==0 || strcmp(stage->opcode, "BNZ")==0) {
            if(stage->buffer != 0) {
                cpu->pc = stage->buffer;
                strcpy(cpu->stage[DRF].opcode,"");
                cpu->stage[DRF].pc=0;
                strcpy(cpu->stage[EX].opcode,"");
                cpu->stage[EX].pc=0;
                cpu->regs_valid[cpu->stage[EX].rd] = 0;
            }
        }


        /* Copy data from decode latch to execute latch*/
        cpu->stage[WB] = cpu->stage[MEM];
		if (ENABLE_DEBUG_MESSAGES)
		{
            print_stage_content("Memory", stage);
        }
	}
  	/*else
	{
		strcpy(stage->opcode,"");
		stage->pc = 1111;
  		cpu->stage[WB] = cpu->stage[MEM];
  		if (ENABLE_DEBUG_MESSAGES)
		{
  			print_stage_content("Memory", stage);
  		}
  	}*/

    else
    {
        cpu->stage[WB] = cpu->stage[MEM];
        //printf("\nMemory\n" );
        if (ENABLE_DEBUG_MESSAGES)
        {
            //print_stage_content("Memory", stage);
            print_stage_contents("Memory");
        }
	}
  return 0;
}

/*
 *  Writeback Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int writeback(APEX_CPU* cpu)
{
  	CPU_Stage* stage = &cpu->stage[WB];
  	if (!stage->busy && !stage->stalled)
	{
		/* Update register file */
        if (strcmp(stage->opcode, "MOVC") == 0)
		{
            cpu->regs[stage->rd] = stage->buffer;
			cpu->regs_valid[stage->rd] = 0;
        }
        //ADD
		if (strcmp(stage->opcode, "ADD") == 0)
		{
            cpu->regs[stage->rd] = stage->buffer;
            cpu->regs_valid[stage->rd]=0;
            if( cpu->regs[stage->rd]==0)
            {
                cpu->zflag=1;
            }
            else
            {
                cpu->zflag=0;
            }
        }

        //LOAD
        if (strcmp(stage->opcode, "LOAD") == 0)
		{
    			cpu->regs[stage->rd] = stage->mem_address;
			cpu->regs_valid[stage->rd] = 0;
        }

        //SUB
        if (strcmp(stage->opcode, "SUB") == 0)
		{
            cpu->regs[stage->rd] = stage->buffer;
            cpu->regs_valid[stage->rd]=0;
            if( cpu->regs[stage->rd]==0)
            {
                cpu->zflag=1;
            }
            else
            {
                cpu->zflag=0;
            }
        }

        //XOR
        if (strcmp(stage->opcode, "XOR") == 0)
		{
            cpu->regs[stage->rd] = stage->buffer;
			cpu->regs_valid[stage->rd]=0;
        }

        //OR
        if (strcmp(stage->opcode, "OR") == 0)
		{
            cpu->regs[stage->rd] = stage->buffer;
			cpu->regs_valid[stage->rd]=0;
        }

        //AND
        if (strcmp(stage->opcode, "AND") == 0)
		{
            cpu->regs[stage->rd] = stage->buffer;
			cpu->regs_valid[stage->rd]=0;
        }

        //MUL
        if (strcmp(stage->opcode, "MUL") == 0)
		{
            cpu->regs[stage->rd] = stage->buffer;
			cpu->regs_valid[stage->rd]=0;
            if( cpu->regs[stage->rd]==0)
            {
                cpu->zflag=1;
            }
            else
            {
                cpu->zflag=0;
            }
        }

        //HALT
		if (strcmp(stage->opcode, "HALT") == 0)
		{
			hck=1;
		}
		if(strcmp(stage->opcode,"") == 0){}else{
        //cpu->regs_valid[stage->rd] = 0;
        cpu->ins_completed++;}
		
		if(strcmp(stage->opcode,"BZ")==0 || strcmp(stage->opcode, "BNZ")==0) {
            if(stage->buffer!=0){
                if(stage->imm<0){
                    cpu->ins_completed = cpu->ins_completed + ((stage->imm/4)-1);
                }else{
                    cpu->ins_completed = cpu->ins_completed - (stage->imm/4);
                }
                //printf("\ncpu->ins_completed after updating: %d\n", cpu->ins_completed);
            }
        }

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Writeback", stage);
        }
    }
	/*else
	{
		if (ENABLE_DEBUG_MESSAGES)
		{
  			print_stage_content("Memory", stage);
  		}
  	}*/
	else
    {
        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_contents("Writeback");
    	}
	}
	//printf("\ncpu->ins_completed: %d\n",cpu->ins_completed);
  	return 0;
}

/*
 *  APEX CPU simulation loop
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
 //Display all register values
void display(APEX_CPU* cpu)
{
	for(int i=0;i<32;i++)
	{
		printf("\n \t R%d || value=%d",i,cpu->regs[i]);
	}
}
void displaymem(APEX_CPU* cpu)
{
	for(int i=0; i<100; i++)
	{
		printf("\n\tMEM_Value[%d] || Value = %d",i,cpu->data_memory[i]);
	}
}

int
APEX_cpu_run(APEX_CPU* cpu)
{
  	while (1)
	{
        /* All the instructions committed, so exit */
        if (cpu->ins_completed == cpu->code_memory_size || hck == 1)
		{
            printf("(apex) >> Simulation Complete\n");
            break;
        }

        if (ENABLE_DEBUG_MESSAGES)
		{
            printf("\t-----------------------------------------------\n");
            printf("\tClock Cycle #: %d\n", cpu->clock);
            printf("\t-----------------------------------------------\n");
        }

        writeback(cpu);
        memory(cpu);
        execute(cpu);
        decode(cpu);
        fetch(cpu);
        cpu->clock++;
  	}
	printf("=============================STATE OF ARCHITECTURAL REGISTER FILE=============================\n");
  	for(int i=0;i<16;i++)
  	{
		printf("\n");
  		printf(" | Register[%d] | Value=%d | status=%s | \n",i,cpu->regs[i],(cpu->regs_valid[i])?"Valid" : "Invalid");

  	}
	printf("====================================STATE OF DATA MEMORY======================================\n");

  	for(int i=0;i<99;i++)
  	{
        printf(" | MEM[%d] | Value=%d | \n",i,cpu->data_memory[i]);
  	}
  	return 0;
}
