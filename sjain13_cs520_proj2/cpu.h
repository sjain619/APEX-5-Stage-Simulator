
#ifndef _APEX_CPU_H_
#define _APEX_CPU_H_
/**
 *  cpu.h
 *  Contains various CPU and Pipeline Data structures
 *
 *  Author :
 *  Gaurav Kothari (gkothar1@binghamton.edu)
 *  State University of New York, Binghamton
 */

enum
{
  F,
  DRF,
  EX,
  MEM,
  WB,
  NUM_STAGES
};

/* Format of an APEX instruction  */
typedef struct APEX_Instruction
{
  char opcode[128]; // Operation Code
  int rd;       // Destination Register Address
  int rs1;        // Source-1 Register Address
  int rs2;        // Source-2 Register Address
  int imm;        // Literal Value
} APEX_Instruction;

struct Register{
  char name[3];
  int value;
  int status;
  struct Flags zero;
};

struct RegisterFile{
  struct Register R[16];
};

struct PhysicalRF{
  struct Register P[32];
  char renamed[32][4];
  bool latest[32];
};

struct ReorderBuffer{
  struct InstructionInfo entry[32];
  char tag[32];
};

typedef struct Queue{
  struct InstructionInfo ins[32];
  struct InstructionInfo get_ins_lsq(int);
  struct InstructionInfo get_ins_from_lsq();
  struct InstructionInfo get_div_from_iq();
  struct InstructionInfo get_int_from_iq();
  struct InstructionInfo get_mul_from_iq();
  struct InstructionInfo get_ins_iq(int);
  struct Queue iq[16];
  struct Queue lsq[32];
}Queue;

// Boolean Type Functions
bool iq_full();
bool lsq_full_if_mem();
bool no_rob_slot(); 
bool iq_has_int(int);
bool iq_has_mul(int);
bool iq_has_div(int);

/* Model of CPU stage latch */
typedef struct CPU_Stage
{
  int pc;       // Program Counter
  char opcode[128]; // Operation Code
  int rs1;        // Source-1 Register Address
  int rs2;        // Source-2 Register Address
  int rd;       // Destination Register Address
  int imm;        // Literal Value
  int rs1_value;  // Source-1 Register Value
  int rs2_value;  // Source-2 Register Value
  int buffer;   // Latch to hold some value
  int mem_address;  // Computed Memory Address
  int busy;       // Flag to indicate, stage is performing some action
  int stalled;    // Flag to indicate, stage is stalled
  int mul_flag;       
  int nop;      // flag for printing nop
  int temp_pc;
  int flush;
  int arithmetic_instr;
  int bubble;
  struct CPU_Stage* next;
  struct CPU_Stage* prev;
  int CPU_Stage* enqueue_rob);
// VOID TYPE FUNCTIONS
void rob_entry_print(struct InstructionInfo*);
void display_rob();
void display_prf();
void print_stage_content_after_rename(struct Stage*);
void print_instruction_after_rename(struct InstructionInfo*);
void display_isq();
void display_lsq();
void iq_init();
void lsq_init();
void enqueue_iq(struct Stage*);
void enqueue_lsq(struct Stage*);
void forward_data_to_lsq(struct Stage*, struct Queue*);
void forward_data_to_iq(struct Stage*, struct Queue*);
void read_broadcasted_reg_iq();
void read_broadcasted_reg_lsq();
void update_rob_tag(struct InstructionInfo*);
void clear_rob();
void clear_cfq();
void dequeue_rob();
void dequeue_iq(struct InstructionInfo*);
void dequeue_lsq(struct InstructionInfo*);


APEX_Instruction*
create_code_memory(const char* filename, int* size);

APEX_CPU*
APEX_cpu_init(const char* filename);

int
APEX_cpu_run(APEX_CPU* cpu);

void
APEX_cpu_stop(APEX_CPU* cpu);

int
fetch(APEX_CPU* cpu);

int
decode(APEX_CPU* cpu);

int
execute(APEX_CPU* cpu);

int
memory(APEX_CPU* cpu);

int
writeback(APEX_CPU* cpu);

#endif