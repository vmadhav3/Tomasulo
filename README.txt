The branch predictor is a digital circuit that predicts the result of a branching instruction before the condition for branching is known. It improves the flow in the instruction pipeline and avoids unnecessary stalls when predictions are correct. Branch predictors play a critical role in achieving high performance in many modern pipelined microprocessor architectures. In this project the design of a simulator for a modern superscalar architecture is outlined. The goal is to develop code for the simulation and study of different branch predictors. We chose three branch prediction algorithms for our project the Always Taken Branch Predictor, the 2-bit Branch Predictor and the Probability based branch predictor. The approach is discussed in the proposed approach section and tabulated results are in the results section.
PROPOSED APPROACH:
Always Taken Branch Predictor:  This is a Static branch predictor. It always predicts the same direction for the same branch during program execution. It is a hardware-fixed mechanism. In this mechanism the branch prediction is always ‘TAKEN’. Always taken uses the branch target address to store and reuse target address. Each time the branch is resolved the corresponding address is stored in the branch target address. When the branch predictor predicts as taken, it retrieves the branch target address from the cache and uses this address to fetch the next instruction. 

Probability based Branch Predictor:  This is a predictor based on probability. A probability function was implemented to determine whether or not the branch is taken or not. For our project we choose if the probability function has a value equal or more than 0.35 the branch prediction outcome is taken if not the predictor outcome is not taken. 
2-bit Branch Predictor:  This predictor is a state machine. It has four states namely Strongly Not Taken, Weakly Not Taken, Weakly Taken, Strongly Taken. When a branch is evaluated, the corresponding state machine is updated. Branches evaluated as not taken decrement the state toward strongly not taken, and branches evaluated as taken increment the state toward strongly taken. A prediction must miss twice before it changes state. 
The architecture is implemented using Tomasulo’s Algorithm. There are three stages of this algorithm. They are Issue, Execute, and Write back. There are reservation stations to fetch and store the instruction and data. There are 3 reservation stations for ADD operation, 2 for MULTIPLICATOIN, and 2 for LOAD instructions. This reservation stations pose a limit on the number of instructions that can be fetched. Stalls can be caused if the reservation stations are full. This algorithm addresses read after write(RAW) dependencies and write after read(WAR) dependencies. 

CODE IMPLEMENTATION:

Step 1: Parse the given file 
Step 2: Store all the instructions, registers and memory locations into proper data structure sequentially
Step 3: Execute instruction by instruction


          PSEUDO CODE
Parse the code  & store it
While(instruction != “HALT”)
{
    Execute Instruction sequentially
    Generate the clock cycle table acc. to Tomasulo      
        Algorithm
    If(instruction= “BRANCH”)
    {Perform branch prediction}
    Instruction++;clock++
}

 

Code generates clock cycle, handles structural hazards, RAW hazards.
