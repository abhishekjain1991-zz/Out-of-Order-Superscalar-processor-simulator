#include "procsim.hpp"
#include<iostream>
#include<vector>
using namespace std;
/**
 * Subroutine for initializing the processor. You many add and initialize any global or heap
 * variables as needed.
 * XXX: You're responsible for completing this routine
 *
 * @r ROB size
 * @k0 Number of k0 FUs
 * @k1 Number of k1 FUs
 * @k2 Number of k2 FUs
 * @f Number of instructions to fetch
 * @m Schedule queue multiplier
 */
proc_inst_t p1;
class iaddress					//address database
{public:
    int32_t i_no;				//instruction number
    uint32_t instructionaddress;		//instruction address
    int32_t opcode;				//opcode
    int32_t destreg;				//Destination registers
    int32_t srcreg[2];				//Source registers
    int32_t s1Ra,s2Ra;				//Ready bits for source registers
    int32_t tags1,tags2;			//tags for both source registers
    int FU_T,FU_C,FU_M;				//fu type,count of cycles spent in that fu
    int i_stg;					//stores the current stage number of the cycle
    int f_cyc,d_cyc,s_cyc,e_cyc,su_cyc,ret_cyc; //stores the cycle in which it went through that stage
						//i create a vector of all instructions currently in the processor thus acting as a database 							//which is then looked up by each stageof the pipeline so that it can operate on it accordingly

iaddress(int32_t num,uint32_t address,int32_t opcd, int32_t dstreg, int32_t src1, int32_t src22,int32_t r1, int32_t r2, int32_t t1, int32_t t2,
int fut, int fuc, int fumc, int stage, int fc, int dc, int sc, int ec, int suc, int retc)
{
    i_no = num;				
    instructionaddress = address;		
    opcode=opcd;				
    destreg=dstreg;
			
    srcreg[0]=src1;	
    srcreg[1]=src22;				
    s1Ra=r1;
    s2Ra=r2;				
    tags1=t1;
    tags2=t2;			
    FU_T=fut;
    FU_C=fuc;
    FU_M=fumc;
    i_stg=stage;				
    f_cyc=fc;
    d_cyc=dc;
    s_cyc=sc;
    e_cyc=ec;
    su_cyc=suc;
    ret_cyc=retc;

    					
}

};


class dispatch
{public:
int32_t inod;
int32_t futype;
dispatch(int32_t add,int32_t fuT) :inod(add),futype(fuT)
{}

};


class fut0
{
public:
uint32_t inofut0;
fut0(int32_t add):inofut0(add)
{};
};

class fut1
{
public:
uint32_t inofut1[2];
fut1(int32_t add)
{inofut1[0]=add;inofut1[1]=add;
}
};

class fut2
{
public:
uint32_t inofut2[3];
fut2(int32_t add)
{inofut2[0]=add;inofut2[1]=add;inofut2[3]=add;
}
};
//no pushbackss for FUS, just copying data only hence no constructor defined, also there will be a vector of vectors for each fu.
class sfut0
{public:
uint32_t shadd0;
sfut0(uint32_t add):shadd0(add)
{}
};
class sfut1
{public:
uint32_t shadd1;
sfut1(uint32_t add):shadd1(add)
{}
};

class sfut2
{public:
uint32_t shadd2;
sfut2(uint32_t add):shadd2(add)
{}
};
class rob
{public:
int32_t inor,rdy;
rob(int32_t add, int32_t redy) :inor(add),rdy(redy)
{}

};

class regf
{public:
uint32_t tag;
int ready;
regf(int32_t t, int32_t r):tag(t),ready(r)
{}
};

class cdbfu0
{public:
int32_t addrcf0;
cdbfu0(int32_t add):addrcf0(add)
{}
};

class cdbfu1
{public:
int32_t addrcf1;
cdbfu1(int32_t add):addrcf1(add)
{}
};

class cdbfu2
{public:
int32_t addrcf2;
cdbfu2(int32_t add):addrcf2(add)
{}
};

vector<iaddress>iadd;					//address database	
vector<dispatch>dq;					//dispatch q
vector<rob>rq;						//robq
vector<fut0>f0;						//functional unit f0	
vector<fut1>f1;						//functional unit f1
vector<fut2>f2;						//functional unit f2
vector<sfut0>sf0;					//schedule q for f0	
vector<sfut1>sf1;					//schedule q for f1
vector<sfut2>sf2;					//schedule q for f2
vector<cdbfu0>cdbf0;					//cdb for f0	
vector<cdbfu1>cdbf1;					//cdb for f1
vector<cdbfu2>cdbf2;					//cdb for f2
vector<regf>rfile;					//registerfile
uint32_t K0,K1,K2;       				//number of units of each type
uint32_t M;						//multiplier
uint32_t F;						//fetch width
uint32_t R;						//robsize;
uint32_t instct=0;					//instruction counter
uint32_t cycnt=0,acycnt=0,icycnt=0;			//cycle counter,retire counter, instruction counter


bool fetch()
{bool rfail=false;
    int32_t i_n;				//instruction number
    uint32_t i_a;				//instruction address
    int32_t opc;				//opcode
    int32_t d_r;				//Destination registers
    int32_t s_r[2];				//Source registers
    int32_t s1r,s2r;				//Ready bits for source registers
    int32_t t1,t2;				//tags for both source registers
    int fut,fuc,fum;				//fu type,count of cycles spent in that fu
    int istg;					//stores the current stage number of the cycle
    int fcyc,dcyc,scyc,ecyc,sucyc,retcyc; 	//stores the cycle in which it went through that stage
						//i create a vector of all instructions currently in the processor thus acting as a database 							//which is then looked up by each stageof the pipeline so that it can operate on it accordingly
						//on retirement of each instruction the database removes that instruction so as to avoid 							//performace issues
int32_t dispatchv;
if((R-dq.size())>F)
dispatchv=F;
else
dispatchv=R-dq.size();
	for(int i=0;i<dispatchv;i++)
		{rfail=read_instruction(&p1);
			if(rfail==false)
			return false;
		instct++;
		i_n=instct;
		i_a=p1.instruction_address;
		opc=p1.op_code;
		d_r=p1.dest_reg;
		s_r[0]=p1.src_reg[0];
		s_r[1]=p1.src_reg[1];
		s1r=0;
		s2r=0;
		t1=0;
		t2=0;
		if(p1.op_code==-1)
		{fut=0;}
		else
		{fut=p1.op_code;}
		fuc=0;	
		fum=fut+1;
		istg=0;
		fcyc=0;dcyc=0;scyc=0;ecyc=0;sucyc=0;retcyc=0;
		iadd.push_back(iaddress(i_n,i_a,opc,d_r,s_r[0],s_r[1],s1r,s2r,t1,t2,fut,fuc,fum,istg,cycnt,dcyc,scyc,ecyc,sucyc,retcyc));
   		dq.push_back(dispatch(i_n,fut));		
		}

return true;


}

void dispatch()
{// assigning cycle count to those instructions in dispatch queue


		for(int i=0;i<dq.size();i++)
		{
		for(int j=0;j<iadd.size();j++)
			{
			if(dq[i].inod==iadd[j].i_no && iadd[j].i_stg<3)  	
				{
				iadd[j].d_cyc=cycnt;//assign the cycle number in which the instruction was in dispatch
				iadd[j].i_stg=3;
				}			
					
			
			}
		}
//check rob and schedule ques, push back in schedule queue and delete the entries from dipatch queue
int count,count1,count2,x,y,z;
		for(int i=0;i<dq.size();)
		{bool dispatchi=false;
			if(rq.size()<R)
				{switch (dq[i].futype)
				       {case 0: 
						if(sf0.size()<(M*K0))
						{sf0.push_back(sfut0(dq[i].inod));
						rq.push_back(rob(dq[i].inod,0));
						for(int j=0;j<iadd.size();j++)
							{
							if(dq[i].inod==iadd[j].i_no)
							{       if(iadd[j].srcreg[0]==-1)
							iadd[j].s1Ra=1;
							else if(rfile[iadd[j].srcreg[0]].ready==1)
							iadd[j].s1Ra=1;
							else
							{iadd[j].s1Ra=0;
							iadd[j].tags1=rfile[iadd[j].srcreg[0]].tag;
							}
			
							if(iadd[j].srcreg[1]==-1)
							iadd[j].s2Ra=1;				
							else if(rfile[iadd[j].srcreg[1]].ready==1)
							iadd[j].s2Ra=1;
							else
							{iadd[j].s2Ra=0;
							iadd[j].tags2=rfile[iadd[j].srcreg[1]].tag;
							}

							
							if(iadd[j].destreg!=-1)
							{						
							rfile[iadd[j].destreg].ready=0;
							rfile[iadd[j].destreg].tag=iadd[j].i_no;
							
							}
														
							break;
					
							}
			
							}		
							dispatchi=true;
							dq.erase(dq.begin());							
							
						}	

						
						
						break;
					case 1: 
						if(sf1.size()<(M*K1))
							{sf1.push_back(sfut1(dq[i].inod));
							rq.push_back(rob(dq[i].inod,0));
							for(int j=0;j<iadd.size();j++)
							{
							if(dq[i].inod==iadd[j].i_no)
							{       if(iadd[j].srcreg[0]==-1)
							iadd[j].s1Ra=1;
							else if(rfile[iadd[j].srcreg[0]].ready==1)
							iadd[j].s1Ra=1;
							else
							{iadd[j].s1Ra=0;
							iadd[j].tags1=rfile[iadd[j].srcreg[0]].tag;
							}
			
							if(iadd[j].srcreg[1]==-1)
							iadd[j].s2Ra=1;				
							else if(rfile[iadd[j].srcreg[1]].ready==1)
							iadd[j].s2Ra=1;
							else
							{iadd[j].s2Ra=0;
							iadd[j].tags2=rfile[iadd[j].srcreg[1]].tag;
							}

							if(iadd[j].destreg!=-1)
							{						
							rfile[iadd[j].destreg].ready=0;
							rfile[iadd[j].destreg].tag=iadd[j].i_no;
							}
														
							break;
					
							}
			
							}
							dispatchi=true;	
							dq.erase(dq.begin());
							}
						
						break;
					case 2: 
						if(sf2.size()<(M*K2))
							{sf2.push_back(sfut2(dq[i].inod));
							rq.push_back(rob(dq[i].inod,0));
							for(int j=0;j<iadd.size();j++)
							{
							if(dq[i].inod==iadd[j].i_no)
							{       if(iadd[j].srcreg[0]==-1)
							iadd[j].s1Ra=1;
							else if(rfile[iadd[j].srcreg[0]].ready==1)
							iadd[j].s1Ra=1;
							else
							{iadd[j].s1Ra=0;
							iadd[j].tags1=rfile[iadd[j].srcreg[0]].tag;
							}
			
							if(iadd[j].srcreg[1]==-1)
							iadd[j].s2Ra=1;				
							else if(rfile[iadd[j].srcreg[1]].ready==1)
							iadd[j].s2Ra=1;
							else
							{iadd[j].s2Ra=0;
							iadd[j].tags2=rfile[iadd[j].srcreg[1]].tag;
							}

							
							if(iadd[j].destreg!=-1)
							{						
							rfile[iadd[j].destreg].ready=0;
							rfile[iadd[j].destreg].tag=iadd[j].i_no;
						
							}
														
							break;
					
							}
			
							}
							dispatchi=true;
							dq.erase(dq.begin());
						
							}
						
						break;
					}
				}
			if(dispatchi==false)
				break;	
		}
}


void schedule()
{//all independent instructions are marked ready to fire
//for schedule q of functional unit of type 0

		for(int j=0;j<sf0.size();j++)
		{
			for(int k=0;k<iadd.size();k++)
			{
			if(iadd[k].i_no==sf0[j].shadd0 && iadd[k].i_stg<4)
					{
						if(iadd[k].s_cyc==0)
						{iadd[k].s_cyc=cycnt;
						}	
						if(iadd[k].s1Ra==1 && iadd[k].s2Ra==1)
						{iadd[k].i_stg=4;
						}
					break;
					}
			}	
		}		
	

//for schedule q of functional unit of type 1

		for(int j=0;j<sf1.size();j++)
		{
			for(int k=0;k<iadd.size();k++)
			{
			if(iadd[k].i_no==sf1[j].shadd1 && iadd[k].i_stg<4)
					{
						if(iadd[k].s_cyc==0)
						{iadd[k].s_cyc=cycnt;
						}	
						if(iadd[k].s1Ra==1 && iadd[k].s2Ra==1)
						{iadd[k].i_stg=4;
						}
					break;
					}
			}	
		}		
	
//for schedule q of functional unit of type 2

		for(int j=0;j<sf2.size();j++)
		{
			for(int k=0;k<iadd.size();k++)
			{
			if(iadd[k].i_no==sf2[j].shadd2 && iadd[k].i_stg<4)
					{
						if(iadd[k].s_cyc==0)
						{iadd[k].s_cyc=cycnt;
						}	
						if(iadd[k].s1Ra==1 && iadd[k].s2Ra==1)
						{iadd[k].i_stg=4;
						}
					break;
					}
			}	
		}		
	
//checking and updating each schedule queue according to cdb broadcast
// for schedule queue of functional unit of type 0


		for(int j=0;j<sf0.size();j++)
		{
			for(int k=0;k<iadd.size();k++)
			{
			if(iadd[k].i_no==sf0[j].shadd0 && iadd[k].i_stg<4)
					{bool finding = false;
						for(int l=0;l<cdbf0.size();l++)
						{
							
							if(iadd[k].tags1==cdbf0[l].addrcf0)
							{iadd[k].s1Ra=1;finding=true;
							}	
							if(iadd[k].tags2==cdbf0[l].addrcf0)
							{iadd[k].s2Ra=1;finding=true;
							}
						}
							
						for(int l=0;l<cdbf1.size();l++)
						{	
							if(iadd[k].tags1==cdbf1[l].addrcf1)
							{iadd[k].s1Ra=1;finding=true;
							}	
							if(iadd[k].tags2==cdbf1[l].addrcf1)
							{iadd[k].s2Ra=1;finding=true;
							}
						}
					
						for(int l=0;l<cdbf2.size();l++)
						{	
							if(iadd[k].tags1==cdbf2[l].addrcf2)
							{iadd[k].s1Ra=1;finding=true;
							}	
							if(iadd[k].tags2==cdbf2[l].addrcf2)
							{iadd[k].s2Ra=1;finding=true;
							}
						}
					
						break;
					
					}
			}//didnt break here coz it may so happen that two instructions might be looking for the same tag	
		}		
	

//for schedule queue of functional unit of type 1


		for(int j=0;j<sf1.size();j++)
		{
			for(int k=0;k<iadd.size();k++)
			{
			if(iadd[k].i_no==sf1[j].shadd1)
					{bool finding = false;
						for(int l=0;l<cdbf0.size();l++)
						{	
							if(iadd[k].tags1==cdbf0[l].addrcf0)
							{iadd[k].s1Ra=1;finding=true;
							}	
							if(iadd[k].tags2==cdbf0[l].addrcf0)
							{iadd[k].s2Ra=1;finding=true;
							}
						}
							
						for(int l=0;l<cdbf1.size();l++)
						{	
							if(iadd[k].tags1==cdbf1[l].addrcf1)
							{iadd[k].s1Ra=1;finding=true;
							}	
							if(iadd[k].tags2==cdbf1[l].addrcf1)
							{iadd[k].s2Ra=1;finding=true;
							}
						}
						
						for(int l=0;l<cdbf2.size();l++)
						{	
							if(iadd[k].tags1==cdbf2[l].addrcf2)
							{iadd[k].s1Ra=1;finding=true;
							}	
							if(iadd[k].tags2==cdbf2[l].addrcf2)
							{iadd[k].s2Ra=1;finding=true;
							}
						}
						
						break;
					}
			}//didnt break here coz it may so happen that two instructions might be looking for the same tag	
		}		
	

//for schedule queue of functional unit of type 2

		for(int j=0;j<sf2.size();j++)
		{
			for(int k=0;k<iadd.size();k++)
			{
			if(iadd[k].i_no==sf2[j].shadd2)
					{bool finding = false;
						for(int l=0;l<cdbf0.size();l++)
						{	
							if(iadd[k].tags1==cdbf0[l].addrcf0)
							{iadd[k].s1Ra=1;finding=true;
							}	
							if(iadd[k].tags2==cdbf0[l].addrcf0)
							{iadd[k].s2Ra=1;finding=true;
							}
						}
							
						for(int l=0;l<cdbf1.size();l++)
						{	
							if(iadd[k].tags1==cdbf1[l].addrcf1)
							{iadd[k].s1Ra=1;finding=true;
							}	
							if(iadd[k].tags2==cdbf1[l].addrcf1)
							{iadd[k].s2Ra=1;finding=true;
							}
						}
						
						for(int l=0;l<cdbf2.size();l++)
						{	
							if(iadd[k].tags1==cdbf2[l].addrcf2)
							{iadd[k].s1Ra=1;finding=true;
							}	
							if(iadd[k].tags2==cdbf2[l].addrcf2)
							{iadd[k].s2Ra=1;finding=true;
							}
						}
						
						break;
					}
			}//didnt break here coz it may so happen that two instructions might be looking for the same tag	
		}		
	

}



void execute()
{

//executing instructions and writing result on result bus

//fu0
for(int i=0;i<f0.size();i++)
{	
	if(f0[i].inofut0==0)
	{
		{bool check0=false;
		for(int j=0;j<sf0.size();j++)
		{
			for( int k=0;k<iadd.size();k++)
			{if(iadd[k].i_no==sf0[j].shadd0 && iadd[k].i_stg==4 && iadd[k].FU_C==0)//checking if instructon in schq is ready 											//and is not already present in the execution unit 	
				{f0[i].inofut0=iadd[k].i_no;		      //put that instruction in execution unit
				 iadd[k].e_cyc=cycnt;			      //assign the cycle number for that instruction
				 iadd[k].FU_C++;			      //count number of cycles spent in execution unit
				 if(iadd[k].FU_C==1)			      //check for max count
					{iadd[k].i_stg=5;                     //if max count reached mark instruction ready for state update
					}
				cdbf0[i].addrcf0=iadd[k].i_no;		      //broadcast result on respective cdb
								
				check0=true;				
				break;				
				}
						
			}
		if(check0)						      //this is done because we dont want 2 instructions ready at same 										      //time to execute in one cycle
		break;
		}
		}
	}
f0[i].inofut0=0;							      //make the fu0 free 
}


//fu1
for(int i=0;i<f1.size();i++)
{	
	if(f1[i].inofut1[0]==0)
	{	for(int j=0;j<sf1.size();j++)
		{bool check0=false;
			for( int k=0;k<iadd.size();k++)
			{if(iadd[k].i_no==sf1[j].shadd1 && iadd[k].i_stg==4 && iadd[k].FU_C==0)//checking if instructon in schq is ready
				{f1[i].inofut1[0]=iadd[k].i_no;		      //put that instruction in execution unit
				 iadd[k].e_cyc=cycnt;			      //assign the cycle number for that instruction
				 iadd[k].FU_C++;			      //count number of cycles spent in execution
				iadd[k].i_stg=10;				 
				check0=true;				
				break;				
				}
						
			}
		if(check0)						      //this is done because we dont want 2 instructions ready at same 										      //time to execute in one cycle
		break;
		}
	}
	for(int l=0;l<iadd.size();l++)
	{if(f1[i].inofut1[1]==iadd[l].i_no)
	 	{iadd[l].FU_C++;
		 if(iadd[l].FU_C==2)
			{iadd[l].i_stg=5;
			}
		
		 cdbf1[i].addrcf1=iadd[l].i_no;
		 break;
		}
	}
f1[i].inofut1[1]=f1[i].inofut1[0];
f1[i].inofut1[0]=0;							      //make the fu1 free 
}
//for fu2
for(int i=0;i<f2.size();i++)
{	
	if(f2[i].inofut2[0]==0)
	{	for(int j=0;j<sf2.size();j++)
		{bool check0=false;
			for( int k=0;k<iadd.size();k++)
			{if(iadd[k].i_no==sf2[j].shadd2 && iadd[k].i_stg==4 && iadd[k].FU_C==0)//checking if instructon in schq is ready
				{f2[i].inofut2[0]=iadd[k].i_no;		      //put that instruction in execution unit
				 iadd[k].e_cyc=cycnt;			      //assign the cycle number for that instruction
				 iadd[k].FU_C++;			      //count number of cycles spent in execution unit
				iadd[k].i_stg=11;
				check0=true;				
				break;				
				}
						
			}
		if(check0)						      //this is done because we dont want 2 instructions ready at same 										      //time to execute in one cycle
		break;
		}
	}
	for(int l=0;l<iadd.size();l++)
	{if(f2[i].inofut2[1]==iadd[l].i_no)
	 	{iadd[l].FU_C++;
		iadd[l].i_stg=12;
		break;
		}
	}
	for(int m=0;m<iadd.size();m++)
	{if(f2[i].inofut2[2]==iadd[m].i_no)
	 	{iadd[m].FU_C++;
		if(iadd[m].FU_C==3)
			{iadd[m].i_stg=5;
			}
		 cdbf2[i].addrcf2=iadd[m].i_no;
		 break;
		}
	}

f2[i].inofut2[2]=f2[i].inofut2[1];
f2[i].inofut2[1]=f2[i].inofut2[0];
f2[i].inofut2[0]=0;							      //make the fu2 free 
}

// updating the register file by using the cdb
//cdb for fu0
		for(int i=0;i<cdbf0.size();i++)
		{if(cdbf0[i].addrcf0!=0)
			{
			for(int j=0;j<rfile.size();j++)
				{if(cdbf0[i].addrcf0==rfile[j].tag)
					{rfile[j].ready=1;
					break;
					}
				}
			}
		}

//cdb for fu1
		for(int i=0;i<cdbf1.size();i++)
		{if(cdbf1[i].addrcf1!=0)
			{
			for(int j=0;j<rfile.size();j++)
				{if(cdbf1[i].addrcf1==rfile[j].tag)
					{rfile[j].ready=1;
					break;
					}
				}
			}
		}
//cdb for fu2
		for(int i=0;i<cdbf2.size();i++)
		{if(cdbf2[i].addrcf2!=0)
			{
			for(int j=0;j<rfile.size();j++)
				{if(cdbf2[i].addrcf2==rfile[j].tag)
					{rfile[j].ready=1;
					break;
					}
				}
			}
		}



}

void stateupdate()
{

for(int i=0;i<iadd.size();i++)
	{if(iadd[i].i_stg==5)
		{switch (iadd[i].FU_T)
					{case 0: 
						{
							for(int k=0;k<sf0.size();k++)
								{if(sf0[k].shadd0==iadd[i].i_no)
									{sf0[k].shadd0=0;
									iadd[i].i_stg=6;
									iadd[i].su_cyc=cycnt;
									break;
									}
								}
						}
						break;
					case 1: 
						{
							for(int k=0;k<sf1.size();k++)
								{if(sf1[k].shadd1==iadd[i].i_no)
									{sf1[k].shadd1=0;
								        iadd[i].i_stg=6;
									iadd[i].su_cyc=cycnt;
									break;
									}
								}
						}
						break;
					case 2: 
						{							
						      for(int k=0;k<sf2.size();k++)							
								{if(sf2[k].shadd2==iadd[i].i_no)
									{sf2[k].shadd2=0;
									iadd[i].i_stg=6;
									iadd[i].su_cyc=cycnt;
									break;
									}
								}
						}
						break;
					}
				
		
		}
	}
}

void erasescheduleq()
{
	vector<sfut0>::iterator It;
	vector<sfut1>::iterator It1;
	vector<sfut2>::iterator It2;
// for fu0

for(It=sf0.begin();It!=sf0.end();)
{
	if((*It).shadd0==0)
		{It = sf0.erase(It);
		}
	else It++;
}

//for fu1
for(It1=sf1.begin();It1!=sf1.end();)
{
	if((*It1).shadd1==0)
		{It1 = sf1.erase(It1);
		}
	else It1++;
}
//for fu2
for(It2=sf2.begin();It2!=sf2.end();)
{
	if((*It2).shadd2==0)
		{It2 = sf2.erase(It2);
		}
	else It2++;
}



}


void retire()
{
	for(int i=0;i<rq.size();i++)
	{bool erase=false;	
		for(int j=0;j<iadd.size();j++)
			{if(rq[i].inor==iadd[j].i_no && iadd[j].i_stg==6)
				{
				 rq[i].inor=0;
				 if(iadd[j].ret_cyc==0)
				 iadd[j].ret_cyc=cycnt;
				 erase=true;
				 break;
				}
		}
	  if(!erase || i==(F-1))
		break;
	}

}


void robcleanup()
{
	for(int i=0;i<rq.size();)
	{
	if(rq[i].inor==0)
		{rq.erase(rq.begin());
		}
	else
	break;
	}
}

void setup_proc(uint64_t r, uint64_t k0, uint64_t k1, uint64_t k2, uint64_t f, uint64_t m) {

for(int i=0;i<k0;i++)
{f0.push_back(fut0(0));
}

for(int i=0;i<k1;i++)
{f1.push_back(fut1(0));
}

for(int i=0;i<k2;i++)
{f2.push_back(fut2(0));
}


	for(int i=0;i<k0;i++)
	{cdbf0.push_back(-10);
	}
	for(int i=0;i<k1;i++)
	{cdbf1.push_back(-10);
	}
	for(int i=0;i<k2;i++)
	{cdbf2.push_back(-10);
	}
		


//sq.resize(k0+k1+k2);
M=m;
K0=k0;
K1=k1;
K2=k2;
F=f;
R=r;
//creating the register file now
	for(int i=0;i<32;i++)
		{rfile.push_back(regf(0,1));
		}

}




/**
 * Subroutine that simulates the processor.
 *   The processor should fetch instructions as appropriate, until all instructions have executed
 * XXX: You're responsible for completing this routine
 *
 * @p_stats Pointer to the statistics structure
 */
void run_proc(proc_stats_t* p_stats) {
bool fnend=true;

while(true)
{cycnt++;


//cout<<"cycnt"<<cycnt<<endl;
retire();
stateupdate();
execute();
schedule();
dispatch();
fnend=fetch();
erasescheduleq();
robcleanup();
for(int i=0;i<iadd.size();)
{

if(iadd[i].ret_cyc!=0)
{if(iadd[i].i_no==100000)
acycnt=iadd[i].ret_cyc;
icycnt++;
cout<<iadd[i].i_no<<"	"<<iadd[i].f_cyc<<"	"<<iadd[i].d_cyc<<"	"<<iadd[i].s_cyc<<"	"<<iadd[i].e_cyc<<"	"<<iadd[i].su_cyc<<"	"<<iadd[i].ret_cyc<<"	"<<endl;				
iadd.erase(iadd.begin());
}
else break;
}
if(icycnt==100000)
break;
}


}

/**
 * Subroutine for cleaning up any outstanding instructions and calculating overall statistics
 * such as average IPC or branch prediction percentage
 * XXX: You're responsible for completing this routine
 *
 * @p_stats Pointer to the statistics structure
 */
void complete_proc(proc_stats_t *p_stats) {
double a,b,c;
a=icycnt;
b=acycnt;
c=a/b;
p_stats->avg_inst_retired=c;
p_stats->retired_instruction=icycnt;
p_stats->cycle_count=acycnt;
}
