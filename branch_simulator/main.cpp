
#include<iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include<cmath>
#include <iomanip>
using namespace std;


void bimodal(int, string);
void gshare(int, int, string);
string hextobin(const string &s);
void predict_branch(int [],int);
int binToDec(string n);
int get_pc(string x, int pc);
int decToBin(int N);
void hybrid(int,int,int,int,string);

int main(int argc , char *argv[]){
// bimodal <m2> <trace_file>
// bimodal <m1> <n> <trace_file>
  int ctr = argc;
  for (int i=1; i<ctr; i++){
    cout<<argv[i]<<"\n";
  }

  string b("bimodal");
  string g("gshare");
  string h("hybrid");


  if (4==ctr && b.compare(argv[1])==0){
    //run bimodal
    int prog_ctr;
    prog_ctr = atoi(argv[2]);
    string trace_file (argv[3]);
    bimodal(prog_ctr,trace_file);




  }else if (5==ctr && g.compare(argv[1])==0){

    //run gshare
    int prog_ctr,gbh_bits;
    prog_ctr = atoi(argv[2]);
    gbh_bits = atoi(argv[3]);
    string trace_file (argv[4]);
    gshare(prog_ctr,gbh_bits,trace_file);

  }else if(ctr==7 && h.compare(argv[1])==0){

    //run hybrid
    int prog_ctr_gshare,prog_ctr_bimodal,gbh_bits,pc_ct;
    pc_ct = atoi(argv[2]);
    prog_ctr_gshare = atoi(argv[3]);
    gbh_bits = atoi(argv[4]);
    prog_ctr_bimodal = atoi(argv[5]);
    string trace_file (argv[6]);

    hybrid(pc_ct,prog_ctr_gshare,gbh_bits,prog_ctr_bimodal,trace_file);

  }

}


void hybrid(int pc_ct,int prog_ctr_gshare,int gbh_bits,int prog_ctr_bimodal,string trace_file){



  int max_idx_hy=pow(2,pc_ct);
  int hy_arr[max_idx_hy];
  for (int i=0;i<max_idx_hy;i++){
    hy_arr[i] = 1;
  }
  ifstream inFile;
  inFile.open(trace_file);

  //BIMODAL
  int max_idx_bm=pow(2,prog_ctr_bimodal);
  int bp_arr[max_idx_bm];
  for (int i=0;i<max_idx_bm;i++){

    bp_arr[i] = 4;
  }

  //GSHARE
    int gbh = 0;
    string gbh_str=to_string(gbh);
    while (gbh_str.length()<gbh_bits){
      gbh_str = '0'+gbh_str;
    }


    int max_idx_gbr=pow(2,prog_ctr_gshare);
    int gs_arr[max_idx_gbr];
    for (int i=0;i<max_idx_gbr;i++){
      gs_arr[i] = 4;
    }

  int predict = 0,mispred = 0;
  string x,y;


  if (!inFile) {
    cerr << "Unable to open file datafile.txt";
    exit(1);   // call system to stop
  }
  ////////////////////////////Bimodal prediction
  while (inFile>>x>>y){
    predict++;
    char act_res;
    act_res=y[0];

    int idx_bimodal = get_pc(x,prog_ctr_bimodal);


    char pred_bimodal ;
    if (bp_arr[idx_bimodal]<4){
      pred_bimodal = 'n';
    }else {
      pred_bimodal = 't';
    }
    //gshare
    string pc_hex_str = hextobin(x);
    int pc_hex_str_len = pc_hex_str.length();

    string pc_idx = pc_hex_str.substr(pc_hex_str_len-2-prog_ctr_gshare,prog_ctr_gshare);


    int pc_idx_len = pc_idx.length();



    string substr_1 = pc_idx.substr(0,pc_idx_len-gbh_bits);
    string substr_2 = pc_idx.substr(pc_idx_len-gbh_bits,gbh_bits);


    // xor the decimal of the gbh string with decimal of substr_2
    int xor_result = binToDec(gbh_str)^binToDec(substr_2);


    //convert the xor result into binary then string
    string xor_attach = to_string(decToBin(xor_result));


    while (xor_attach.length()<gbh_bits){
      xor_attach = '0'+xor_attach;
    }


    // attach the string to the substr_1 and get that index
    string final_idx_str = substr_1 + xor_attach;
    int idx_gshare = binToDec(final_idx_str);



    char pred_gshare ;
    if (gs_arr[idx_gshare]<4){
      pred_gshare = 'n';
    }else {
      pred_gshare = 't';
    }

    int idx_hybrid = get_pc(x,pc_ct);
    char pred_hybrid;

    if(hy_arr[idx_hybrid]>=2){
        pred_hybrid = pred_gshare;

        if(act_res == 't' ){
            if (gs_arr[idx_gshare]<7){
              gs_arr[idx_gshare]++;
            }
        }else{
          if (gs_arr[idx_gshare]!=0){
            gs_arr[idx_gshare]--;
          }
        }



    }else{
      pred_hybrid = pred_bimodal;
      if(act_res == 't' ){
          if (bp_arr[idx_bimodal]<7){
            bp_arr[idx_bimodal]++;
          }
      }else{
        if (bp_arr[idx_bimodal]!=0){
          bp_arr[idx_bimodal]--;
        }
      }
    }


    if  (pred_hybrid!=act_res){
      mispred++;
    }
    // update the gbh and gbh_str simultaenously
    string new_gbh_str = gbh_str.substr(0,gbh_bits-1);
    // cout<<new_gbh_str<<"\t";
    if (act_res == 't'){

      gbh_str = '1'+new_gbh_str;
    }else
    {gbh_str = '0'+new_gbh_str;}

    while (gbh_str.length()<gbh_bits){
      gbh_str = '0'+gbh_str;
    }
    // cout<<gbh_str<<"\n";
    gbh = stoi(gbh_str);



    // gshare correct bimodal incorrect --> ++
    if (pred_gshare == act_res && pred_bimodal != act_res){
        if (hy_arr[idx_hybrid] <3){
          hy_arr[idx_hybrid]++;
        }
    }// bimodal correct gshare incorrect --> --
    else if (pred_gshare != act_res && pred_bimodal == act_res){
      if(hy_arr[idx_hybrid]>0){
        hy_arr[idx_hybrid]--;
      }
    }


    //


  }
  double rate = (mispred)/(predict/100000);
  cout<<"number of predictions:\t\t"<<predict<<"\n";
  cout<<"number of mispredictions:\t"<<mispred<<"\n";
  cout<<"misprediction rate:     "<<double(ceil(rate/10)/100)<<"%\n";
  cout<<"FINAL CHOOSER CONTENTS\n";
  for(int i=0;i<max_idx_hy;i++){
    cout<<i<<"\t"<<hy_arr[i]<<"\n";
  }
  cout<<"FINAL GSHARE CONTENTS\n";
  for(int i=0;i<max_idx_gbr;i++){
    cout<<i<<"\t"<<gs_arr[i]<<"\n";
  }
  cout<<"FINAL BIMODAL CONTENTS\n";
  for(int i=0;i<max_idx_bm;i++){
    cout<<i<<"\t"<<bp_arr[i]<<"\n";
  }

  inFile.close();



}












/////////////////////////////////////////////////////////////////////////////////////////////// DOnt change below this
void gshare(int pc ,int gbh_bits ,string trace_file){
  ifstream inFile;
  inFile.open(trace_file);

  int gbh = 0;
  string gbh_str=to_string(gbh);
  while (gbh_str.length()<gbh_bits){
    gbh_str = '0'+gbh_str;
  }


  int max_idx_gbr=pow(2,pc);
  int bp_arr[max_idx_gbr];
  for (int i=0;i<max_idx_gbr;i++){
    bp_arr[i] = 4;
  }

  int predict = 0,mispred = 0;
  string x,y;


  if (!inFile) {
    cerr << "Unable to open file datafile.txt";
    exit(1);   // call system to stop
  }

// int max=0;
  while (inFile>>x>>y){
    // max++;
    // if (max==100)
    //   exit(1);

    char act_res;
    act_res=y[0];

//function call removed
    string pc_hex_str = hextobin(x);
    int pc_hex_str_len = pc_hex_str.length();

    string pc_idx = pc_hex_str.substr(pc_hex_str_len-2-pc,pc);


    int pc_idx_len = pc_idx.length();



    string substr_1 = pc_idx.substr(0,pc_idx_len-gbh_bits);
    string substr_2 = pc_idx.substr(pc_idx_len-gbh_bits,gbh_bits);

    // cout<<pc_idx<<"\t"<<substr_1<<"\t"<<substr_2<<"\n";
    // int test=1;
    // cout<<"gbh_str  "<<test<<" decimal substr_2  "<<binToDec(substr_2)<<"\n";

    // xor the decimal of the gbh string with decimal of substr_2
    int xor_result = binToDec(gbh_str)^binToDec(substr_2);


    //convert the xor result into binary then string
    string xor_attach = to_string(decToBin(xor_result));


    while (xor_attach.length()<gbh_bits){
      xor_attach = '0'+xor_attach;
    }


    // attach the string to the substr_1 and get that index
    string final_idx_str = substr_1 + xor_attach;
    int idx = binToDec(final_idx_str);

    // cout<<idx<<'\t'<<bp_arr[idx]<<"\t";


    // and perform predict function
    char pred ;
    if (bp_arr[idx]<4){
      pred = 'n';
    }else {
      pred = 't';
    }


    predict++;
    if (act_res!=pred){
      mispred++;
    }

    if(act_res == 't' ){
        if (bp_arr[idx]<7){
          bp_arr[idx]++;
        }
    }else{
      if (bp_arr[idx]!=0){
        bp_arr[idx]--;
      }
    }

    // cout<<pred<<"\t"<<bp_arr[idx]<<"\t";


    // update the gbh and gbh_str simultaenously
    string new_gbh_str = gbh_str.substr(0,gbh_bits-1);
    // cout<<new_gbh_str<<"\t";
    if (act_res == 't'){

      gbh_str = '1'+new_gbh_str;
    }else
    {gbh_str = '0'+new_gbh_str;}

    while (gbh_str.length()<gbh_bits){
      gbh_str = '0'+gbh_str;
    }
    // cout<<gbh_str<<"\n";
    gbh = stoi(gbh_str);


  }

  double rate = (mispred)/(predict/100000);
  cout<<"number of predictions:\t\t"<<predict<<"\n";
  cout<<"number of mispredictions:\t"<<mispred<<"\n";
  cout<<"misprediction rate:     "<<double(ceil(rate/10)/100)<<"%\n";
  cout<<"FINAL GSHARE CONTENTS\n";
  for(int i=0;i<max_idx_gbr;i++){
    cout<<i<<"\t"<<bp_arr[i]<<"\n";
  }

  inFile.close();
}





void bimodal(int pc, string trace_file){
  ifstream inFile;
  inFile.open(trace_file);
  int max_idx_bm=pow(2,pc);
  int bp_arr[max_idx_bm];
  for (int i=0;i<max_idx_bm;i++){

    bp_arr[i] = 4;
  }

  int predict = 0,mispred = 0;
  string x,y;


  if (!inFile) {
    cerr << "Unable to open file datafile.txt";
    exit(1);   // call system to stop
  }


  while (inFile>>x>>y){
    char act_res;
    act_res=y[0];

    int idx = get_pc(x,pc);

    // cout<<idx<<"\n";


    char pred ;
    if (bp_arr[idx]<4){
      pred = 'n';
    }else {
      pred = 't';
    }

    predict++;
    if (act_res!=pred){
      mispred++;
    }

    if(act_res == 't' ){
        if (bp_arr[idx]<7){
          bp_arr[idx]++;
        }
    }else{
      if (bp_arr[idx]!=0){
        bp_arr[idx]--;
      }
    }

    // predict_branch(bp_arr,idx);



  }

  double rate = (mispred)/(predict/100000);
  cout<<"number of predictions:\t\t"<<predict<<"\n";
  cout<<"number of mispredictions:\t"<<mispred<<"\n";
  cout<<"misprediction rate:     "<<double(ceil(rate/10)/100)<<"%\n";
cout<<"FINAL BIMODAL CONTENTS\n";
for(int i=0;i<max_idx_bm;i++){
  cout<<i<<"\t"<<bp_arr[i]<<"\n";
}

inFile.close();

}







////////////////////////////////////////////////////////////////////////////////Helper functions
int decToBin(int N)
{

    // To store the binary number
    unsigned long long B_Number = 0;
    int cnt = 0;
    while (N != 0) {
        int rem = N % 2;
        unsigned long long c = pow(10, cnt);
        B_Number += rem * c;
        N /= 2;

        // Count used to store exponent value
        cnt++;
    }

    return B_Number;
}


int get_pc(string x,int pc){

  string pc_hex_str = hextobin(x);
  int pc_hex_str_len = pc_hex_str.length();

  string pc_idx = pc_hex_str.substr(pc_hex_str_len-2-pc,pc);
  int idx = binToDec(pc_idx);
  return idx;
}

void predict_branch(int bp_arr[],int idx){
  // char pred ;
  // if (bp_arr[idx]<4){
  //   pred = 'n';
  // }else {
  //   pred = 't';
  // }
  //
  // predict++;
  // if (act_res!=pred){
  //   mispred++;
  // }
  //
  // if(act_res == 't' ){
  //     if (bp_arr[idx]<7){
  //       bp_arr[idx]++;
  //     }
  // }else{
  //   if (bp_arr[idx]!=0){
  //     bp_arr[idx]--;
  //   }
  // }
}

int binToDec(string n)
{
    string num = n;
    int dec_value = 0;

    // Initializing base value to 1, i.e 2^0
    int base = 1;

    int len = num.length();
    for (int i = len - 1; i >= 0; i--) {
        if (num[i] == '1')
            dec_value += base;
        base = base * 2;
    }

    return dec_value;
}

string hextobin(const string &s){
    string out;
    for(auto i: s){
        uint8_t n;
        if(i <= '9' and i >= '0')
            n = i - '0';
        else
            n = 10 + i - 'A';
        for(int8_t j = 3; j >= 0; --j)
            out.push_back((n & (1<<j))? '1':'0');
    }

    return out;
}
