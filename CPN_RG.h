//
// Created by hecong on 19-12-10.
//

#ifndef CPN_PNML_PARSE_CPN_RG_H
#define CPN_PNML_PARSE_CPN_RG_H

#include "CPN.h"
#include "base.h"
#include <cmath>
#include <ctime>
#include <iomanip>
using namespace std;

#define H1FACTOR 13
#define CPNRGTABLE_SIZE 1048576
#define random(x) rand()%(x)

extern NUM_t placecount;
extern NUM_t transitioncount;
extern NUM_t varcount;
extern CPN *cpnet;
extern bool timeflag;
class MarkingMeta;
class CPN_RGNode;
class CPN_RG;
class CPN_Product_Automata;

void MarkingMetacopy(MarkingMeta &mm1,const MarkingMeta &mm2,type tid,SORTID sid);
int MINUS(MarkingMeta &mm1,const MarkingMeta &mm2);
void PLUS(MarkingMeta &mm1,const MarkingMeta &mm2);

class Bindind
{
public:
    COLORID *varvec;
    Bindind *next;
    MarkingMeta *arcsMultiSet;

    Bindind() {
        next=NULL;
        arcsMultiSet = NULL;
        varvec=new COLORID[30];
        memset(varvec,MAXSHORT,sizeof(COLORID)*30);
    }
    ~Bindind() {
        delete [] varvec;
        //delete [] arcsMultiSet;
        MallocExtension::instance()->ReleaseFreeMemory();
    }
};

class MarkingMeta
{
private:
    Tokens *tokenQ;      //带有头结点的队列；
    SHORTNUM colorcount;
    type tid;
    SORTID sid;
public:
    index_t hashvalue;

    MarkingMeta(){tokenQ=new Tokens;colorcount=0;hashvalue=0;}
    ~MarkingMeta(){
        Tokens *p,*q;
        p=tokenQ;
        while(p)
        {
            q=p->next;
            delete p;
            p=q;
        }
        MallocExtension::instance()->ReleaseFreeMemory();
    }
    void initiate(type t,SORTID s) {tid=t;sid=s;}
    void insert(Tokens *token);
    index_t Hash();
    bool operator>=(const MarkingMeta &mm);
    NUM_t Tokensum();
    void printToken();
    friend class CPN_RGNode;
    friend class CPN_RG;
    friend void MarkingMetacopy(MarkingMeta &mm1,const MarkingMeta &mm2,type tid,SORTID sid);
    friend int MINUS(MarkingMeta &mm1,const MarkingMeta &mm2);
    friend void PLUS(MarkingMeta &mm1,const MarkingMeta &mm2);
};

class TranBindQueue
{
public:
    unsigned short tranid;
    Bindind *bindQ;
    Bindind *rear;
    TranBindQueue *next;

    TranBindQueue() {
        tranid=MAXSHORT;
        bindQ=NULL;
        rear=NULL;
        next=NULL;
    }
    ~TranBindQueue() {
        Bindind *q,*p;
        p=bindQ;
        while(p)
        {
            q=p->next;
            delete p->arcsMultiSet;
            delete p;
            p=q;
        }
        MallocExtension::instance()->ReleaseFreeMemory();
    }
    void insert(Bindind *bound);
    void getSize(int &size);
//    bool AllBound() {return bindptr==NULL?true:false;}
};

class FiTranQueue
{
public:
    TranBindQueue *fireQ;

    FiTranQueue(){fireQ=NULL;}
    ~FiTranQueue() {
        TranBindQueue *p,*q;
        p=fireQ;
        while(p)
        {
            q=p->next;
            delete p;
            p=q;
        }
        MallocExtension::instance()->ReleaseFreeMemory();
    }
    void insert(TranBindQueue *TBQ);
    void getSize(int &size);
    bool isfirable(string transname) const;
//    bool AllFired() {return (fireptr==NULL)?true:false;}
};

class CPN_RGNode
{
public:
    MarkingMeta *marking;
    CPN_RGNode *next;
    FiTranQueue *firetrans;
public:
    int numid;
    CPN_RGNode(){
        numid = 0;
        marking=new MarkingMeta[placecount];
        for(int i=0;i<placecount;++i) {
            marking[i].initiate(cpnet->place[i].tid,cpnet->place[i].sid);
        }
        next=NULL;
        firetrans = new FiTranQueue;
    }
    ~CPN_RGNode();
    index_t Hash(SHORTNUM *weight);
    bool operator==(const CPN_RGNode &n1);
    void operator=(const CPN_RGNode &rgnode);
    void printMarking();
    void selfcheck();
    friend class CPN_RG;
};

class CPN_RG
{
private:
    CPN_RGNode **markingtable;
    CPN_RGNode *initnode;
    bool colorflag;
public:
    NUM_t nodecount;
    SHORTNUM *weight;
    NUM_t hash_conflict_times;
public:
    CPN_RG();
    ~CPN_RG();
    void addRGNode(CPN_RGNode *mark);
    void getFireableTranx(CPN_RGNode *curnode,FiTranQueue &fireableTs);
    void getTranxBinding(CPN_RGNode *curnode,const CPN_Transition &tt,int level,Bindind *&bind,TranBindQueue &tbq);
    //void giveArcColor(multiset_node *expnode,CPN_RGNode *curnode,VARID *varvec,index_t placeidx,int psindex);
    //int giveVarColor(multiset_node *expnode,const MarkingMeta &mm,VARID *varvec,int psindex=0);
    //void computeArcEXP(const arc_expression &arcexp,MarkingMeta &mm,int psnum=0);
    void computeArcEXP(const arc_expression &arcexp,MarkingMeta &mm,COLORID *varcolors,int psnum=0);
    //void computeArcEXP(meta *expnode,MarkingMeta &mm,int psnum=0);
    void computeArcEXP(meta *expnode,MarkingMeta &mm,COLORID *varcolors,int psnum=0);
    //void getTupleColor(meta *expnode,COLORID *cid,int ptr);
    void getTupleColor(meta *expnode,COLORID *cid,COLORID *varcolors,int ptr);
    void judgeGuard(CTN *node,COLORID *cid);
    CPN_RGNode *CPNRGinitialnode();
    CPN_RGNode *CPNRGcreatenode(CPN_RGNode *mark,const TranBindQueue *tbs,const Bindind *bindptr,bool &exist);
    void Generate(CPN_RGNode *mark);
    bool NodeExist(CPN_RGNode *mark,CPN_RGNode *&existmark);
    friend class CPN_Product_Automata;
};
#endif //CPN_PNML_PARSE_CPN_RG_H
