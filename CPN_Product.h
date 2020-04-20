//
// Created by hecong on 20-1-8.
//

#ifndef CPN_PNML_PARSE_CPN_PRODUCT_H
#define CPN_PNML_PARSE_CPN_PRODUCT_H

#include "CPN_RG.h"
#include "BA/SBA.h"
#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <pthread.h>

using namespace std;
#define UNREACHABLE 0xffffffff
#define CPSTACKSIZE 33554432  //2^22
#define CHASHSIZE 1048576  //2^20
#define each_ltl_time 300

extern CPN_RG *cpnRG;

class CHashtable;
class CPstack;
class CPN_Product_Automata;
class Small_CPN_Product;
void sig_handler(int num);

struct fire_info
{
    const TranBindQueue *fireptr = NULL;
    const Bindind *bindptr = NULL;
};

class CPN_Product
{
private:
    index_t id;
    CPN_RGNode *RGname_ptr;
    int BAname_id;
    index_t stacknext;

    //non-recurrent extra info
    SArcNode *pba;
    fire_info fi;

public:
    CPN_Product() {
        id = 0;
        RGname_ptr = NULL;
        BAname_id = -1;
        stacknext = UNREACHABLE;
        pba = NULL;
    }
    ~CPN_Product() {};
    void initiate() {
        fi.fireptr = RGname_ptr->firetrans->fireQ;
        if(fi.fireptr!=NULL)
            fi.bindptr = fi.fireptr->bindQ;
    }
    friend class CHashtable;
    friend class CPstack;
    friend class CPN_Product_Automata;
    friend class Small_CPN_Product;
};

class Small_CPN_Product
{
public:
    CPN_RGNode *RGname_ptr;
    int BAname_id;
    Small_CPN_Product *hashnext;
public:
    Small_CPN_Product(CPN_Product *node){
        this->RGname_ptr = node->RGname_ptr;
        this->BAname_id = node->BAname_id;
        hashnext = NULL;
    }
};

class CHashtable
{
public:
    Small_CPN_Product **table;
    NUM_t nodecount;
    NUM_t hash_conflict_times;

public:
    CHashtable() {
        table = new Small_CPN_Product*[CHASHSIZE];
        for(int i=0;i<CHASHSIZE;++i){
            table[i] = NULL;
        }
        nodecount = 0;
        hash_conflict_times = 0;
    }
    ~CHashtable() {
        for(int i=0;i<CHASHSIZE;++i) {
            if(table[i]!=NULL)
            {
                Small_CPN_Product *p=table[i];
                Small_CPN_Product *q;
                while(p!=NULL) {
                    q = p->hashnext;
                    delete p;
                    p = q;
                }
            }
        }
        delete [] table;
        MallocExtension::instance()->ReleaseFreeMemory();
    }
    index_t hashfunction(CPN_Product *node);
    index_t hashfunction(Small_CPN_Product *node);
    void insert(CPN_Product *node);
    Small_CPN_Product *search(CPN_Product *node);
    void remove(CPN_Product *node);
    void resetHash();
};

class CPstack
{
public:
    CPN_Product **mydata;
    index_t *hashlink;
    index_t toppoint;

     CPstack();
     ~CPstack();
     index_t hashfunction(CPN_Product *node);
     CPN_Product *top();
     CPN_Product *pop();
     CPN_Product *search(CPN_Product *node);
     int push(CPN_Product *node);
     NUM_t size();
     bool empty();
     void clear();
};

class CPN_Product_Automata
{
private:
    vector<CPN_Product> initial_status;
    CHashtable h;
    CStack<index_t> astack;
    CStack<index_t> dstack;
    CPstack cstack;
    SBA *ba;
    thread detect_mem_thread;
    int ret;
    bool result;
    bool memory_flag;
    bool stack_flag;
public:
    CPN_Product_Automata(SBA *sba);
    ~CPN_Product_Automata();
    bool isLabel(CPN_RGNode *state,int sj);
    bool judgeF(string s);
    bool handleLTLF(string s, CPN_RGNode *state);
    bool handleLTLC(string s, CPN_RGNode *state);
    void handleLTLCstep(NUM_t &front_sum, NUM_t &latter_sum, string s, CPN_RGNode *state);
    NUM_t sumtoken(string s,CPN_RGNode *state);
    CPN_Product *getNextChild(CPN_Product *curstate);
    void TCHECK(CPN_Product *p0);
    void UPDATE(CPN_Product *p0);
    int PUSH(CPN_Product *p0);
    void POP();
    void getinitial_status(CPN_RGNode *initnode);
    void getProduct();
    void ModelChecker(string propertyid);
    int getresult(){return ret;}
    void detect_memory();
};
#endif //CPN_PNML_PARSE_CPN_PRODUCT_H
