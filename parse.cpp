#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <string>
#include <algorithm>
#include <iomanip>

#define NUM_REGISTERS 16
#define MAIN_MEM_SIZE 1024

using namespace std;

void parse(string file);
int reg_to_index(string s);
bool takenPrediction();
bool twoBitPrediction(bool pred[2]);
bool randomPrediction();

struct Register
{
    int read_busy;
    int write_busy;
    float data;
};

struct Instruction
{
  int issue;
  int exec_complete;
  int write_back;
  vector<string> instruct;
};

int main(int argc, char** argv)
{
    string s;

    if(argc < 2)
        cerr << "Incorrect usage!" << endl;
    else
        s = argv[1];

    srand(time(NULL));
    parse(s);
}

void parse(string file)
{
    vector<Instruction> instr;
    Instruction temp;
    int cot = 0, numline = -1;
    std::string::size_type sz;
    string line,sub, command;
    ifstream myfile;
    myfile.open(file.c_str());

    // Create registers and set them all to 0
    Register regs[NUM_REGISTERS];
    for(int i = 0; i < NUM_REGISTERS; i++)
    {
        regs[i].data = 1;
        regs[i].read_busy = 1;
        regs[i].write_busy = 1;
    }

    int branch_miss = 0;
    int branch_hit = 0;
    bool arr[2];
    arr[0] = true;
    arr[1] = true;

    // Create a main memomry array
    float main_mem[MAIN_MEM_SIZE];
    for(int i = 0; i < MAIN_MEM_SIZE; i++)
        main_mem[i] = 0;


    while ( getline (myfile,line) )
    {
        istringstream iss(line);
        string word;
        while(iss >> word)
        {
            if(word[0] =='-' && word[1]=='-')
                break;
            if(cot==0)
            {
                numline=atoi(word.c_str());
            }
            else
            {
                temp.instruct.push_back(word);
                while(getline(iss, word, ','))
                {
                    stringstream reg_stream(word);
                    reg_stream >> word;
                    temp.instruct.push_back(word);
                }
            }
            cot++;
        }
        if(instr.size() < numline && temp.instruct.size() > 0)
            instr.push_back(temp);
        else if(instr.size() == numline)
            break;
        temp.instruct.clear();
    }

    // Read number of main memory entries
    int index;
    float data;
    while(getline(myfile, line))
    {
        stringstream ss(line);
        string word;
        ss >> word;
        if(word[0] == '<')
        {
            sscanf(line.c_str(), "<%i> <%f>", &index, &data);
            main_mem[index] = data;
        }
    }

    //Run instructions
    int instr_count = 0;
    int clock = 1;

    //initialization for no. of adders and multipliers
    int addreg[3]={0,0,0};
    int mulreg[2]={0,0};
    int intreg[2]={0,0};

    while(1)
    {
        temp = instr[instr_count];
        command = temp.instruct[0];
        stall:

        if(command == "FPADD")
        {
            if(clock > addreg[0])
            {
            instr[instr_count].issue = clock;
            if(temp.instruct[3][0] == '#')
            {
                if(regs[reg_to_index(temp.instruct[1])].write_busy <= clock &&
                   regs[reg_to_index(temp.instruct[2])].read_busy <= clock)
                {
                    regs[reg_to_index(temp.instruct[1])].data = regs[reg_to_index(temp.instruct[2])].data +
                                                                     atof(temp.instruct[3].substr(1).c_str());

                    instr[instr_count].exec_complete = clock + 4;
                    instr[instr_count].write_back = clock + 5;

                    regs[reg_to_index(temp.instruct[1])].write_busy = clock + 5;
                    regs[reg_to_index(temp.instruct[1])].read_busy = clock + 5;
                    addreg[0] = instr[instr_count].write_back;
                    sort(addreg, addreg + 3);
                }
                else if(regs[reg_to_index(temp.instruct[2])].read_busy > clock)
                {
                    regs[reg_to_index(temp.instruct[1])].data = regs[reg_to_index(temp.instruct[2])].data +
                                                                atof(temp.instruct[3].substr(1).c_str());

                    instr[instr_count].exec_complete = regs[reg_to_index(temp.instruct[1])].write_busy + 3;
                    instr[instr_count].write_back = regs[reg_to_index(temp.instruct[1])].write_busy + 4;

                    regs[reg_to_index(temp.instruct[1])].write_busy += 4;
                    regs[reg_to_index(temp.instruct[1])].read_busy += 4;
                    addreg[0]=instr[instr_count].write_back; //check here
                    sort(addreg, addreg + 3);
                }
            }
            else if(regs[reg_to_index(temp.instruct[1])].write_busy &&
                    regs[reg_to_index(temp.instruct[2])].read_busy <= clock &&
                    regs[reg_to_index(temp.instruct[3])].read_busy <= clock)
            {
                regs[reg_to_index(temp.instruct[1])].data = regs[reg_to_index(temp.instruct[2])].data +
                                                                 regs[reg_to_index(temp.instruct[3])].data;

                instr[instr_count].exec_complete = clock + 4;
                instr[instr_count].write_back = clock + 5;

                regs[reg_to_index(temp.instruct[1])].write_busy = clock + 5;
                regs[reg_to_index(temp.instruct[1])].read_busy = clock + 5;
                addreg[0]=instr[instr_count].write_back;
                sort(addreg, addreg + 3);
            }
            else if(regs[reg_to_index(temp.instruct[2])].read_busy > clock ||
                    regs[reg_to_index(temp.instruct[3])].read_busy > clock)
            {
                regs[reg_to_index(temp.instruct[1])].data = regs[reg_to_index(temp.instruct[2])].data +
                                                            regs[reg_to_index(temp.instruct[3])].data;

                instr[instr_count].exec_complete =
                    (regs[reg_to_index(temp.instruct[2])].read_busy > regs[reg_to_index(temp.instruct[3])].read_busy ?
                     regs[reg_to_index(temp.instruct[2])].read_busy :
                     regs[reg_to_index(temp.instruct[3])].read_busy) + 3;
                instr[instr_count].write_back =
                    (regs[reg_to_index(temp.instruct[2])].read_busy > regs[reg_to_index(temp.instruct[3])].read_busy ?
                     regs[reg_to_index(temp.instruct[2])].read_busy :
                     regs[reg_to_index(temp.instruct[3])].read_busy) + 4;

                regs[reg_to_index(temp.instruct[1])].write_busy += 4;
                regs[reg_to_index(temp.instruct[1])].read_busy =
                    (regs[reg_to_index(temp.instruct[2])].read_busy > regs[reg_to_index(temp.instruct[3])].read_busy ?
                     regs[reg_to_index(temp.instruct[2])].read_busy :
                     regs[reg_to_index(temp.instruct[3])].read_busy) + 4;
                     addreg[0]=instr[instr_count].write_back; //check here
					 sort(addreg, addreg + 3);
            }
        }
             else
        {
            clock++;
            goto stall;
        }
        }
        else if(command == "FPSUB")
        {
            if(clock > addreg[0])
            {
            instr[instr_count].issue = clock;
            if(temp.instruct[3][0] == '#')
            {
                if(regs[reg_to_index(temp.instruct[1])].write_busy <= clock &&
                   regs[reg_to_index(temp.instruct[2])].read_busy <= clock)
                {
                    regs[reg_to_index(temp.instruct[1])].data = regs[reg_to_index(temp.instruct[2])].data -
                                                                atof(temp.instruct[3].substr(1).c_str());

                    instr[instr_count].exec_complete = clock + 4;
                    instr[instr_count].write_back = clock + 5;

                    regs[reg_to_index(temp.instruct[1])].write_busy = clock + 5;
                    regs[reg_to_index(temp.instruct[1])].read_busy = clock + 5;
                    addreg[0]=instr[instr_count].write_back;
                    sort(addreg, addreg + 3);
                }
                else if(regs[reg_to_index(temp.instruct[2])].read_busy > clock)
                {
                    regs[reg_to_index(temp.instruct[1])].data = regs[reg_to_index(temp.instruct[2])].data -
                                                                atof(temp.instruct[3].substr(1).c_str());

                    instr[instr_count].exec_complete = regs[reg_to_index(temp.instruct[1])].write_busy + 3;
                    instr[instr_count].write_back = regs[reg_to_index(temp.instruct[1])].write_busy + 4;

                    regs[reg_to_index(temp.instruct[1])].write_busy += 4;
                    regs[reg_to_index(temp.instruct[1])].read_busy += 4;
                    addreg[0]=instr[instr_count].write_back; //check here
                    sort(addreg, addreg + 3);
                }
            }
            else if(regs[reg_to_index(temp.instruct[1])].write_busy &&
                    regs[reg_to_index(temp.instruct[2])].read_busy <= clock &&
                    regs[reg_to_index(temp.instruct[3])].read_busy <= clock)
            {
                regs[reg_to_index(temp.instruct[1])].data = regs[reg_to_index(temp.instruct[2])].data -
                                                            regs[reg_to_index(temp.instruct[3])].data;

                instr[instr_count].exec_complete = clock + 4;
                instr[instr_count].write_back = clock + 5;

                regs[reg_to_index(temp.instruct[1])].write_busy = clock + 5;
                regs[reg_to_index(temp.instruct[1])].read_busy = clock + 5;
                addreg[0]=instr[instr_count].write_back;
                sort(addreg, addreg + 3);
            }
            else if(regs[reg_to_index(temp.instruct[2])].read_busy > clock ||
                    regs[reg_to_index(temp.instruct[3])].read_busy > clock)
            {
                regs[reg_to_index(temp.instruct[1])].data = regs[reg_to_index(temp.instruct[2])].data -
                                                            regs[reg_to_index(temp.instruct[3])].data;

                instr[instr_count].exec_complete =
                    (regs[reg_to_index(temp.instruct[2])].read_busy > regs[reg_to_index(temp.instruct[3])].read_busy ?
                     regs[reg_to_index(temp.instruct[2])].read_busy :
                     regs[reg_to_index(temp.instruct[3])].read_busy) + 3;
                instr[instr_count].write_back =
                    (regs[reg_to_index(temp.instruct[2])].read_busy > regs[reg_to_index(temp.instruct[3])].read_busy ?
                     regs[reg_to_index(temp.instruct[2])].read_busy :
                     regs[reg_to_index(temp.instruct[3])].read_busy) + 4;

                regs[reg_to_index(temp.instruct[1])].write_busy += 4;
                regs[reg_to_index(temp.instruct[1])].read_busy =
                    (regs[reg_to_index(temp.instruct[2])].read_busy > regs[reg_to_index(temp.instruct[3])].read_busy ?
                     regs[reg_to_index(temp.instruct[2])].read_busy :
                     regs[reg_to_index(temp.instruct[3])].read_busy) + 4;
                     addreg[0]=instr[instr_count].write_back; //check here
					 sort(addreg, addreg + 3);
            }
        }
            else
        {
            clock++;
            goto stall;
        }
        }
        else if(command == "FPMULT")
        {
            if(clock > mulreg[0])
            {
            instr[instr_count].issue = clock;
            if(temp.instruct[3][0] == '#')
            {
                if(regs[reg_to_index(temp.instruct[1])].write_busy <= clock &&
                   regs[reg_to_index(temp.instruct[2])].read_busy <= clock)
                {
                    regs[reg_to_index(temp.instruct[1])].data = regs[reg_to_index(temp.instruct[2])].data *
                                                                atof(temp.instruct[3].substr(1).c_str());


                    if(atof(temp.instruct[3].substr(1).c_str()) != -1 && atof(temp.instruct[3].substr(1).c_str()) != 1
                       && atof(temp.instruct[3].substr(1).c_str()) != 0 && atof(temp.instruct[3].substr(1).c_str()))
                    {
                        instr[instr_count].exec_complete = clock + 6;
                        instr[instr_count].write_back = clock + 7;

                        regs[reg_to_index(temp.instruct[1])].write_busy = clock + 7;
                        regs[reg_to_index(temp.instruct[1])].read_busy = clock + 7;
                        mulreg[0] = instr[instr_count].write_back;
                        sort(mulreg, mulreg + 2);
                    }
                    else
                    {
                        instr[instr_count].exec_complete = clock + 2;
                        instr[instr_count].write_back = clock + 3;

                        regs[reg_to_index(temp.instruct[1])].write_busy = clock + 3;
                        regs[reg_to_index(temp.instruct[1])].read_busy = clock + 3;
                        mulreg[0] = instr[instr_count].write_back;
                        sort(mulreg, mulreg + 2);
                    }
                }
                else if(regs[reg_to_index(temp.instruct[2])].read_busy > clock)
                {
                    regs[reg_to_index(temp.instruct[1])].data = regs[reg_to_index(temp.instruct[2])].data *
                                                                atof(temp.instruct[3].substr(1).c_str());

                    if(atof(temp.instruct[3].substr(1).c_str()) != -1 && atof(temp.instruct[3].substr(1).c_str()) != 1
                       && atof(temp.instruct[3].substr(1).c_str()) != 0 && atof(temp.instruct[3].substr(1).c_str()))
                    {
                        instr[instr_count].exec_complete = regs[reg_to_index(temp.instruct[1])].write_busy + 5;
                        instr[instr_count].write_back = regs[reg_to_index(temp.instruct[1])].write_busy + 6;

                        regs[reg_to_index(temp.instruct[1])].write_busy =
                                regs[reg_to_index(temp.instruct[1])].write_busy + 6;
                        regs[reg_to_index(temp.instruct[1])].read_busy += 6;
                        mulreg[0] = instr[instr_count].write_back; //check here
                        sort(mulreg, mulreg + 2);
                    }
                    else
                    {
                        instr[instr_count].exec_complete = regs[reg_to_index(temp.instruct[1])].write_busy + 2;
                        instr[instr_count].write_back = regs[reg_to_index(temp.instruct[1])].write_busy + 3;

                        regs[reg_to_index(temp.instruct[1])].write_busy =
                                regs[reg_to_index(temp.instruct[1])].write_busy + 3;
                        regs[reg_to_index(temp.instruct[1])].read_busy += 3;
                        mulreg[0] = instr[instr_count].write_back; //check here
                        sort(mulreg, mulreg + 2);
                    }
                }
            }
            else if(regs[reg_to_index(temp.instruct[1])].write_busy &&
                    regs[reg_to_index(temp.instruct[2])].read_busy <= clock &&
                    regs[reg_to_index(temp.instruct[3])].read_busy <= clock)
            {
                regs[reg_to_index(temp.instruct[1])].data = regs[reg_to_index(temp.instruct[2])].data *
                                                            regs[reg_to_index(temp.instruct[3])].data;

                instr[instr_count].exec_complete = clock + 6;
                instr[instr_count].write_back = clock + 7;

                regs[reg_to_index(temp.instruct[1])].write_busy = clock + 7;
                regs[reg_to_index(temp.instruct[1])].read_busy = clock + 7;
                mulreg[0]=instr[instr_count].write_back;
                    sort(mulreg, mulreg + 2);
            }
            else if(regs[reg_to_index(temp.instruct[2])].read_busy > clock ||
                    regs[reg_to_index(temp.instruct[3])].read_busy > clock)
            {
                regs[reg_to_index(temp.instruct[1])].data = regs[reg_to_index(temp.instruct[2])].data *
                                                            regs[reg_to_index(temp.instruct[3])].data;

                instr[instr_count].exec_complete =
                    (regs[reg_to_index(temp.instruct[2])].read_busy > regs[reg_to_index(temp.instruct[3])].read_busy ?
                     regs[reg_to_index(temp.instruct[2])].read_busy :
                     regs[reg_to_index(temp.instruct[3])].read_busy) + 5;
                instr[instr_count].write_back =
                    (regs[reg_to_index(temp.instruct[2])].read_busy > regs[reg_to_index(temp.instruct[3])].read_busy ?
                     regs[reg_to_index(temp.instruct[2])].read_busy :
                     regs[reg_to_index(temp.instruct[3])].read_busy) + 6;

                regs[reg_to_index(temp.instruct[1])].write_busy += 6;
                regs[reg_to_index(temp.instruct[1])].read_busy =
                    (regs[reg_to_index(temp.instruct[2])].read_busy > regs[reg_to_index(temp.instruct[3])].read_busy ?
                     regs[reg_to_index(temp.instruct[2])].read_busy :
                     regs[reg_to_index(temp.instruct[3])].read_busy) + 6;
                     mulreg[0]=instr[instr_count].write_back; //check here
					 sort(mulreg, mulreg + 2);
            }
        }

          else
        {
            clock++;
            goto stall;
        }
        }
        else if(command == "FPDIV")
        {
            if(clock > mulreg[0])
            {
            instr[instr_count].issue = clock;
            if(temp.instruct[3][0] == '#')
            {
                if(regs[reg_to_index(temp.instruct[1])].write_busy <= clock &&
                   regs[reg_to_index(temp.instruct[2])].read_busy <= clock)
                {
                    regs[reg_to_index(temp.instruct[1])].data = regs[reg_to_index(temp.instruct[2])].data /
                                                                atof(temp.instruct[3].substr(1).c_str());

                    instr[instr_count].exec_complete = clock + 9;
                    instr[instr_count].write_back = clock + 10;

                    regs[reg_to_index(temp.instruct[1])].write_busy = clock + 10;
                    regs[reg_to_index(temp.instruct[1])].read_busy = clock + 10;
                    mulreg[0]=instr[instr_count].write_back;
                    sort(mulreg, mulreg + 2);
                }
                else if(regs[reg_to_index(temp.instruct[2])].read_busy > clock)
                {
                    regs[reg_to_index(temp.instruct[1])].data = regs[reg_to_index(temp.instruct[2])].data /
                                                                atof(temp.instruct[3].substr(1).c_str());

                    instr[instr_count].exec_complete = regs[reg_to_index(temp.instruct[2])].write_busy + 8;
                    instr[instr_count].write_back = regs[reg_to_index(temp.instruct[2])].write_busy + 9;

                    regs[reg_to_index(temp.instruct[1])].read_busy = regs[reg_to_index(temp.instruct[2])].read_busy + 9;
                    regs[reg_to_index(temp.instruct[2])].read_busy += 9;
                    mulreg[0]=instr[instr_count].write_back; //check here
					 sort(mulreg, mulreg + 2);
                }
            }
            else if(regs[reg_to_index(temp.instruct[1])].write_busy &&
                    regs[reg_to_index(temp.instruct[2])].read_busy <= clock &&
                    regs[reg_to_index(temp.instruct[3])].read_busy <= clock)
            {
                regs[reg_to_index(temp.instruct[1])].data = regs[reg_to_index(temp.instruct[2])].data /
                                                            regs[reg_to_index(temp.instruct[3])].data;

                instr[instr_count].exec_complete = clock + 9;
                instr[instr_count].write_back = clock + 10;

                regs[reg_to_index(temp.instruct[1])].write_busy = clock + 10;
                regs[reg_to_index(temp.instruct[1])].read_busy = clock + 10;
                mulreg[0]=instr[instr_count].write_back;
                sort(mulreg, mulreg + 2);
            }
            else if(regs[reg_to_index(temp.instruct[2])].read_busy > clock ||
                    regs[reg_to_index(temp.instruct[3])].read_busy > clock)
            {
                regs[reg_to_index(temp.instruct[1])].data = regs[reg_to_index(temp.instruct[2])].data /
                                                            regs[reg_to_index(temp.instruct[3])].data;

                instr[instr_count].exec_complete =
                    (regs[reg_to_index(temp.instruct[2])].read_busy > regs[reg_to_index(temp.instruct[3])].read_busy ?
                     regs[reg_to_index(temp.instruct[2])].read_busy :
                     regs[reg_to_index(temp.instruct[3])].read_busy) + 8;
                instr[instr_count].write_back =
                    (regs[reg_to_index(temp.instruct[2])].read_busy > regs[reg_to_index(temp.instruct[3])].read_busy ?
                     regs[reg_to_index(temp.instruct[2])].read_busy :
                     regs[reg_to_index(temp.instruct[3])].read_busy) + 9;

                regs[reg_to_index(temp.instruct[1])].write_busy += 9;
                regs[reg_to_index(temp.instruct[1])].read_busy =
                    (regs[reg_to_index(temp.instruct[2])].read_busy > regs[reg_to_index(temp.instruct[3])].read_busy ?
                     regs[reg_to_index(temp.instruct[2])].read_busy :
                     regs[reg_to_index(temp.instruct[3])].read_busy) + 9;
                     mulreg[0]=instr[instr_count].write_back; //check here
					 sort(mulreg, mulreg + 2);
            }
        }
            else
        {
            clock++;
            goto stall;
        }

        }
        else if(command == "ADD")
        {
            if(clock > intreg[0])
            {
            instr[instr_count].issue = clock;
            if(temp.instruct[3][0] == '#')
            {
                if(regs[reg_to_index(temp.instruct[1])].write_busy <= clock &&
                   regs[reg_to_index(temp.instruct[2])].read_busy <= clock)
                {
                    regs[reg_to_index(temp.instruct[1])].data = regs[reg_to_index(temp.instruct[2])].data +
                                                                atof(temp.instruct[3].substr(1).c_str());

                    instr[instr_count].exec_complete = clock + 2;
                    instr[instr_count].write_back = clock + 3;

                    regs[reg_to_index(temp.instruct[1])].write_busy = clock + 3;
                    regs[reg_to_index(temp.instruct[1])].read_busy = clock + 3;
                    intreg[0]=instr[instr_count].write_back;
                    sort(intreg, intreg + 2);
                }
                else if(regs[reg_to_index(temp.instruct[2])].read_busy > clock)
                {
                    regs[reg_to_index(temp.instruct[1])].data = regs[reg_to_index(temp.instruct[2])].data +
                                                                atof(temp.instruct[3].substr(1).c_str());

                    instr[instr_count].exec_complete = regs[reg_to_index(temp.instruct[1])].write_busy + 1;
                    instr[instr_count].write_back = regs[reg_to_index(temp.instruct[1])].write_busy + 2;

                    regs[reg_to_index(temp.instruct[1])].write_busy += 2;
                    regs[reg_to_index(temp.instruct[1])].read_busy += 2;
                    intreg[0]=instr[instr_count].write_back; //check here
                    sort(intreg, intreg + 2);
                }
            }
            else if(regs[reg_to_index(temp.instruct[1])].write_busy &&
                    regs[reg_to_index(temp.instruct[2])].read_busy <= clock &&
                    regs[reg_to_index(temp.instruct[3])].read_busy <= clock)
            {
                regs[reg_to_index(temp.instruct[1])].data = regs[reg_to_index(temp.instruct[2])].data +
                                                            regs[reg_to_index(temp.instruct[3])].data;

                instr[instr_count].exec_complete = clock + 2;
                instr[instr_count].write_back = clock + 3;

                regs[reg_to_index(temp.instruct[1])].write_busy = clock + 3;
                regs[reg_to_index(temp.instruct[1])].read_busy = clock + 3;
                intreg[0]=instr[instr_count].write_back;
                sort(intreg, intreg + 2);
            }
            else if(regs[reg_to_index(temp.instruct[2])].read_busy > clock ||
                    regs[reg_to_index(temp.instruct[3])].read_busy > clock)
            {
                regs[reg_to_index(temp.instruct[1])].data = regs[reg_to_index(temp.instruct[2])].data +
                                                            regs[reg_to_index(temp.instruct[3])].data;

                instr[instr_count].exec_complete =
                    (regs[reg_to_index(temp.instruct[2])].read_busy > regs[reg_to_index(temp.instruct[3])].read_busy ?
                     regs[reg_to_index(temp.instruct[2])].read_busy :
                     regs[reg_to_index(temp.instruct[3])].read_busy) + 1;
                instr[instr_count].write_back =
                    (regs[reg_to_index(temp.instruct[2])].read_busy > regs[reg_to_index(temp.instruct[3])].read_busy ?
                     regs[reg_to_index(temp.instruct[2])].read_busy :
                     regs[reg_to_index(temp.instruct[3])].read_busy) + 2;

                regs[reg_to_index(temp.instruct[1])].write_busy += 2;
                regs[reg_to_index(temp.instruct[1])].read_busy =
                    (regs[reg_to_index(temp.instruct[2])].read_busy > regs[reg_to_index(temp.instruct[3])].read_busy ?
                     regs[reg_to_index(temp.instruct[2])].read_busy :
                     regs[reg_to_index(temp.instruct[3])].read_busy) + 2;
                     intreg[0]=instr[instr_count].write_back; //check here
                    sort(intreg, intreg + 2);
            }
        }
            else
        {
            clock++;
            goto stall;
        }
        }
        else if(command == "SUB")
        {
            if(clock > intreg[0])
            {
            instr[instr_count].issue = clock;
            if(temp.instruct[3][0] == '#')
            {
                if(regs[reg_to_index(temp.instruct[1])].write_busy <= clock &&
                   regs[reg_to_index(temp.instruct[2])].read_busy <= clock)
                {
                    regs[reg_to_index(temp.instruct[1])].data = regs[reg_to_index(temp.instruct[2])].data -
                                                                atof(temp.instruct[3].substr(1).c_str());

                    instr[instr_count].exec_complete = clock + 2;
                    instr[instr_count].write_back = clock + 3;

                    regs[reg_to_index(temp.instruct[1])].write_busy = clock + 3;
                    regs[reg_to_index(temp.instruct[1])].read_busy = clock + 3;
                    intreg[0]=instr[instr_count].write_back;
                    sort(intreg, intreg + 2);
                }
                else if(regs[reg_to_index(temp.instruct[2])].read_busy > clock)
                {
                    regs[reg_to_index(temp.instruct[1])].data = regs[reg_to_index(temp.instruct[2])].data -
                                                                atof(temp.instruct[3].substr(1).c_str());

                    instr[instr_count].exec_complete = regs[reg_to_index(temp.instruct[1])].write_busy + 1;
                    instr[instr_count].write_back = regs[reg_to_index(temp.instruct[1])].write_busy + 2;

                    regs[reg_to_index(temp.instruct[1])].write_busy += 2;
                    regs[reg_to_index(temp.instruct[1])].read_busy += 2;
                    intreg[0]=instr[instr_count].write_back; //check here
                    sort(intreg, intreg + 2);
                }
            }
            else if(regs[reg_to_index(temp.instruct[1])].write_busy &&
                    regs[reg_to_index(temp.instruct[2])].read_busy <= clock &&
                    regs[reg_to_index(temp.instruct[3])].read_busy <= clock)
            {
                regs[reg_to_index(temp.instruct[1])].data = regs[reg_to_index(temp.instruct[2])].data -
                                                            regs[reg_to_index(temp.instruct[3])].data;

                instr[instr_count].exec_complete = clock + 2;
                instr[instr_count].write_back = clock + 3;

                regs[reg_to_index(temp.instruct[1])].write_busy = clock + 3;
                regs[reg_to_index(temp.instruct[1])].read_busy = clock + 3;
                intreg[0]=instr[instr_count].write_back;
                sort(intreg, intreg + 2);
            }
            else if(regs[reg_to_index(temp.instruct[2])].read_busy > clock ||
                    regs[reg_to_index(temp.instruct[3])].read_busy > clock)
            {
                regs[reg_to_index(temp.instruct[1])].data = regs[reg_to_index(temp.instruct[2])].data -
                                                            regs[reg_to_index(temp.instruct[3])].data;

                instr[instr_count].exec_complete =
                    (regs[reg_to_index(temp.instruct[2])].read_busy > regs[reg_to_index(temp.instruct[3])].read_busy ?
                     regs[reg_to_index(temp.instruct[2])].read_busy :
                     regs[reg_to_index(temp.instruct[3])].read_busy) + 1;
                instr[instr_count].write_back =
                    (regs[reg_to_index(temp.instruct[2])].read_busy > regs[reg_to_index(temp.instruct[3])].read_busy ?
                     regs[reg_to_index(temp.instruct[2])].read_busy :
                     regs[reg_to_index(temp.instruct[3])].read_busy) + 2;

                regs[reg_to_index(temp.instruct[1])].write_busy += 2;
                regs[reg_to_index(temp.instruct[1])].read_busy =
                    (regs[reg_to_index(temp.instruct[2])].read_busy > regs[reg_to_index(temp.instruct[3])].read_busy ?
                     regs[reg_to_index(temp.instruct[2])].read_busy :
                     regs[reg_to_index(temp.instruct[3])].read_busy) + 2;
                     intreg[0]=instr[instr_count].write_back; //check here
                    sort(intreg, intreg + 2);
            }
        }
            else
        {
            clock++;
            goto stall;
        }
        }
        else if(command == "LOAD")
        {
            if(clock > intreg[0])
            {
            instr[instr_count].issue = clock;
            if(temp.instruct[2][0] == '#')
            {
                regs[reg_to_index(temp.instruct[1])].data = atof(temp.instruct[2].c_str());
                if(regs[reg_to_index(temp.instruct[2])].read_busy <= clock)
                {
                    instr[instr_count].exec_complete = clock + 2;
                    instr[instr_count].write_back = clock + 3;

                    regs[reg_to_index(temp.instruct[1])].read_busy = clock + 3;

                    regs[reg_to_index(temp.instruct[1])].write_busy = clock + 3;
                    regs[reg_to_index(temp.instruct[2])].write_busy = clock + 3;
                    intreg[0]=instr[instr_count].write_back;
                    sort(intreg, intreg + 2);
                }
                else
                {
                    instr[instr_count].exec_complete = regs[reg_to_index(temp.instruct[1])].read_busy + 1;
                    instr[instr_count].write_back = regs[reg_to_index(temp.instruct[1])].read_busy + 2;

                    regs[reg_to_index(temp.instruct[1])].read_busy += 2;

                    regs[reg_to_index(temp.instruct[1])].write_busy = regs[reg_to_index(temp.instruct[1])].read_busy + 2;
                    regs[reg_to_index(temp.instruct[2])].write_busy = regs[reg_to_index(temp.instruct[1])].read_busy + 2;
                    intreg[0]=instr[instr_count].write_back; //check here
                    sort(intreg, intreg + 2);
                }
            }
            else if(temp.instruct[2][0] != 'R')
            {
                regs[reg_to_index(temp.instruct[1])].data = main_mem[atoi(temp.instruct[2].c_str())];
                instr[instr_count].exec_complete =regs[reg_to_index(temp.instruct[1])].write_busy+2;
                instr[instr_count].write_back =regs[reg_to_index(temp.instruct[1])].write_busy+3;

                regs[reg_to_index(temp.instruct[1])].write_busy +=3;
                intreg[0]=instr[instr_count].write_back; //check here
                    sort(intreg, intreg + 2);

            }
            else
            {
                regs[reg_to_index(temp.instruct[1])].data =
                        main_mem[static_cast<int>(regs[reg_to_index(temp.instruct[2])].data)];
                if(regs[reg_to_index(temp.instruct[2])].read_busy <= clock)
                {
                    instr[instr_count].exec_complete = clock + 2;
                    instr[instr_count].write_back = clock + 3;

                    regs[reg_to_index(temp.instruct[1])].read_busy = clock + 3;

                    regs[reg_to_index(temp.instruct[1])].write_busy = clock + 3;
                    regs[reg_to_index(temp.instruct[2])].write_busy = clock + 3;
                    intreg[0]=instr[instr_count].write_back;
                    sort(intreg, intreg + 2);
                }
                else
                {
                    instr[instr_count].exec_complete = regs[reg_to_index(temp.instruct[1])].read_busy + 1;
                    instr[instr_count].write_back = regs[reg_to_index(temp.instruct[1])].read_busy + 2;

                    regs[reg_to_index(temp.instruct[1])].read_busy += 3;

                    regs[reg_to_index(temp.instruct[1])].write_busy = regs[reg_to_index(temp.instruct[1])].read_busy + 2;
                    regs[reg_to_index(temp.instruct[2])].write_busy = regs[reg_to_index(temp.instruct[1])].read_busy + 2;
                    intreg[0]=instr[instr_count].write_back; //check here
                    sort(intreg, intreg + 2);
                }
            }
        }
            else
        {
            clock++;
            goto stall;
        }
        }
        else if(command == "MOV")
        {
            if(clock > intreg[0])
            {
            instr[instr_count].issue = clock;
            regs[reg_to_index(temp.instruct[1])].data = regs[reg_to_index(temp.instruct[2])].data;
            if(regs[reg_to_index(temp.instruct[2])].read_busy <= clock)
            {
                instr[instr_count].exec_complete = clock + 2;
                instr[instr_count].write_back = clock + 3;

                regs[reg_to_index(temp.instruct[1])].read_busy = clock + 3;

                regs[reg_to_index(temp.instruct[1])].write_busy = clock + 3;
                regs[reg_to_index(temp.instruct[2])].write_busy = clock + 3;
                intreg[0]=instr[instr_count].write_back;
                sort(intreg, intreg + 2);
            }
            else
            {
                instr[instr_count].exec_complete = regs[reg_to_index(temp.instruct[1])].read_busy + 1;
                instr[instr_count].write_back = regs[reg_to_index(temp.instruct[1])].read_busy + 2;

                regs[reg_to_index(temp.instruct[1])].read_busy += 2;

                regs[reg_to_index(temp.instruct[1])].write_busy = regs[reg_to_index(temp.instruct[1])].read_busy + 2;
                regs[reg_to_index(temp.instruct[2])].write_busy = regs[reg_to_index(temp.instruct[1])].read_busy + 2;
                intreg[0]=instr[instr_count].write_back; //check here
                sort(intreg, intreg + 2);
            }
        }
            else
        {
            clock++;
            goto stall;
        }
        }
        else if(command == "STR")
        {
            if(clock > intreg[0])
            instr[instr_count].issue = clock;
            main_mem[static_cast<int>(regs[reg_to_index(temp.instruct[1])].data)] =
                    regs[reg_to_index(temp.instruct[2])].data;
            if(regs[reg_to_index(temp.instruct[2])].read_busy <= clock)
            {
                instr[instr_count].exec_complete = clock + 2;
                instr[instr_count].write_back = clock + 3;

                regs[reg_to_index(temp.instruct[1])].read_busy = clock + 3;

                regs[reg_to_index(temp.instruct[1])].write_busy = clock + 3;
                regs[reg_to_index(temp.instruct[2])].write_busy = clock + 3;
                intreg[0]=instr[instr_count].write_back;
                sort(intreg, intreg + 2);
            }
            else
            {
                instr[instr_count].exec_complete = regs[reg_to_index(temp.instruct[1])].read_busy + 1;
                instr[instr_count].write_back = regs[reg_to_index(temp.instruct[1])].read_busy + 2;

                regs[reg_to_index(temp.instruct[1])].read_busy += 2;

                regs[reg_to_index(temp.instruct[1])].write_busy = regs[reg_to_index(temp.instruct[1])].read_busy + 2;
                regs[reg_to_index(temp.instruct[2])].write_busy = regs[reg_to_index(temp.instruct[1])].read_busy + 2;
                intreg[0]=instr[instr_count].write_back; //check here
                sort(intreg, intreg + 2);
            }
        }
        else if(command == "BR")
        {
            instr[instr_count].issue = clock;

            // Perform instruction
            if(regs[reg_to_index(temp.instruct[1])].data > 0)
            {
                for (int i = 0; i < regs[reg_to_index(temp.instruct[1])].data - 1; i++)
                {
                    instr.insert(instr.begin() + instr_count, instr[instr_count + i]);
                }
            }
            else
            {
                for(int i = regs[reg_to_index(temp.instruct[1])].data; i < 0; i++)
                {
                    instr.insert(instr.begin() + instr_count, instr[instr_count + i]);
                }
            }

            // Find out when its done
            if(regs[reg_to_index(temp.instruct[1])].read_busy <= clock)
            {

                instr[instr_count].exec_complete = clock + 2;
                instr[instr_count].write_back = clock + 3;
            }
            else
            {
                instr[instr_count].exec_complete = regs[reg_to_index(temp.instruct[1])].read_busy + 2;
                instr[instr_count].write_back = regs[reg_to_index(temp.instruct[1])].read_busy + 3;
            }

            clock++;
        }
        else if(command == "BNEZ")
        {
            instr[instr_count].issue = clock;

            if(regs[reg_to_index(temp.instruct[1])].data != 0)
            {
                if(atoi(temp.instruct[2].substr(1).c_str()) > 0)
                {
                    for (int i = 0; i < atoi(temp.instruct[2].substr(1).c_str()) - 1; i++)
                    {
                        instr.insert(instr.begin() + instr_count + 1 + i, instr[instr_count + i]);
                    }
                }
                else
                {
                    for(int i = abs(atoi(temp.instruct[2].substr(1).c_str())) - 1; i >= 0; i--)
                    {
                        instr.insert(instr.begin() + instr_count + 1 + (abs(atoi(temp.instruct[2].substr(1).c_str())) - 1 - i), instr[instr_count - i]);
                    }
                }
            }
            if(twoBitPrediction(arr) == (regs[reg_to_index(temp.instruct[1])].data != 0))
            {
                branch_hit++;
                if(!arr[0] && !arr[1])
                {
                    arr[1] = true;
                }
                else if(!arr[0] && arr[1])
                {
                    arr[0] = true;
                    arr[1] = false;
                }
                else if(arr[0] && !arr[1])
                {
                    arr[1] = true;
                }
            }
            else
            {
                branch_miss++;
                if (arr[0] && arr[1])
                {
                    arr[1] = false;
                }
                else if (arr[0] && !arr[1])
                {
                    arr[0] = false;
                    arr[1] = true;
                }
                else if (!arr[0] && arr[1])
                {
                    arr[0] = true;
                    arr[1] = false;
                }
                else if(!arr[0] && !arr[1])
                {
                    arr[1] = true;
                }
            }

            // Timings
            if(regs[reg_to_index(temp.instruct[1])].read_busy <= clock)
            {
                instr[instr_count].exec_complete = clock + 2;
                instr[instr_count].write_back = clock + 3;
            }
            else
            {
                instr[instr_count].exec_complete = regs[reg_to_index(temp.instruct[1])].read_busy + 1;
                instr[instr_count].write_back = instr[instr_count].exec_complete + 1;
            }

            clock++;
        }
        else if(command == "HALT")
        {
            break;
        }
        instr_count++;
        clock++;
    }

    cout << left << setw(20) << setfill(' ') << "Instruction";
    cout << left << setw(20) << setfill(' ') << "Issue";
    cout << left << setw(20) << setfill(' ') << "Execution";
    cout << left << setw(20) << setfill(' ') << "Write Back" << endl;
    for(size_t i = 0; i < instr.size() - 1; i++)
    {
        cout << left << setw(20) << setfill(' ') << instr[i].instruct[0];
        cout << left << setw(20) << setfill(' ') << instr[i].issue;
        cout << left << setw(20) << setfill(' ') << instr[i].exec_complete;
        cout << left << setw(20) << setfill(' ') << instr[i].write_back << endl;
    }
    cout << endl;

    cout << setw(20) << setfill(' ') << "Register";
    cout << setw(20) << setfill(' ') << "Value" << endl;
    for(int i = 0; i < NUM_REGISTERS; i++)
    {
        cout << setw(20) << setfill(' ') << i;
        cout << left << setw(20) << setfill(' ') << regs[i].data << endl;
    }
    cout << endl;

    cout << "Branch miss: " << branch_miss << endl;
    cout << "Branch hit: " << branch_hit << endl;
}

int reg_to_index(string s)
{
    return atoi(s.substr(1).c_str());
}

bool takenPrediction()
{
    return true;
}

bool twoBitPrediction(bool pred[2])
{
    bool taken;
    if (pred[0] && pred[1])
    {
        taken = true;
    }
    else if (pred[0] && !pred[1])
    {
        taken = true;
    }
    else if (!pred[0] && pred[1])
    {
        taken = false;
    }
    else if (!pred[0] && !pred[1])
    {
        taken = false;
    }
    return taken;
}

bool randomPrediction()
{
    return rand() % 100 >= 35;
}



