//
// Created by hecong on 19-12-10.
//

#include "CPN_RG.h"

/*
 * */
void MarkingMetacopy(MarkingMeta &mm1,const MarkingMeta &mm2,type tid,SORTID sid)
{
    mm1.colorcount = mm2.colorcount;
    Tokens *q = mm2.tokenQ->next;
    Tokens *p,*ppre = mm1.tokenQ;
    if(mm2.tid == productsort)
    {
        while(q)
        {
            p = new Tokens;
            int sortnum = sorttable.productsort[mm2.sid].sortnum;
            Tokenscopy(*p,*q,tid,sortnum);
            ppre->next = p;
            ppre = p;
            q=q->next;
        }
    }
    else
    {
        while(q)
        {
            p = new Tokens;
            Tokenscopy(*p,*q,tid);
            ppre->next = p;
            ppre = p;
            q=q->next;
        }
    }
}

int MINUS(MarkingMeta &mm1,const MarkingMeta &mm2)
{
    Tokens *pp1,*pp2,*pp1p;
    pp1 = mm1.tokenQ->next;
    pp1p = mm1.tokenQ;
    pp2 = mm2.tokenQ->next;
    if(mm1.tid == dot)
    {
        if(pp1->tokencount<pp2->tokencount)
            return 0;
        pp1->tokencount = pp1->tokencount-pp2->tokencount;
        if(pp1->tokencount == 0)
        {
            delete pp1;
            mm1.tokenQ->next=NULL;
            mm1.colorcount = 0;
        }
        return 1;
    }
    else if(mm1.tid == usersort)
    {
        COLORID cid1,cid2;
        while(pp2)
        {
            pp2->tokens->getColor(cid2);
            while(pp1)
            {
                pp1->tokens->getColor(cid1);
                if(cid1<cid2)
                {
                    pp1p = pp1;
                    pp1=pp1->next;
                    continue;
                }
                else if(cid1==cid2)
                {
                    if(pp1->tokencount < pp2->tokencount)
                        return 0;
                    pp1->tokencount = pp1->tokencount - pp2->tokencount;
                    if(pp1->tokencount == 0)
                    {
                        pp1p->next = pp1->next;
                        mm1.colorcount--;
                        delete pp1;

                        pp2=pp2->next;
                        pp1=pp1p->next;
                        break;
                    } else{
                        pp2=pp2->next;
                        pp1p = pp1;
                        pp1=pp1->next;
                        break;
                    }
                }
                else if(cid1 > cid2)
                {
                    return 0;
//                    cerr<<"[MINUS]ERROR:The minued is not greater than the subtractor."<<endl;
//                    exit(-1);
                }
            }
            if(pp1==NULL && pp2!=NULL)
            {
                return 0;
            }
        }
        return 1;
    }
    else if(mm1.tid == productsort)
    {
        int sortnum = sorttable.productsort[mm1.sid].sortnum;
        COLORID *cid1,*cid2;
        cid1 = new COLORID[sortnum];
        cid2 = new COLORID[sortnum];
        while(pp2)
        {
            pp2->tokens->getColor(cid2,sortnum);
            while(pp1)
            {
                pp1->tokens->getColor(cid1,sortnum);
                if(array_less(cid1,cid2,sortnum))
                {
                    pp1p = pp1;
                    pp1=pp1->next;
                    continue;
                }
                else if(array_equal(cid1,cid2,sortnum))
                {
                    if(pp1->tokencount<pp2->tokencount)
                        return 0;
                    pp1->tokencount = pp1->tokencount-pp2->tokencount;
                    if(pp1->tokencount == 0)
                    {
                        pp1p->next = pp1->next;
                        mm1.colorcount--;
                        delete pp1;

                        pp1=pp1p->next;
                        pp2=pp2->next;
                        break;
                    }
                    else
                    {
                        pp1p=pp1;
                        pp1=pp1->next;
                        pp2=pp2->next;
                        break;
                    }
                }
                else if(array_greater(cid1,cid2,sortnum))
                {
                    delete [] cid1;
                    delete [] cid2;
                    return 0;
//                    cerr<<"[MINUS]ERROR:The minued is not greater than the subtractor."<<endl;
//                    exit(-1);
                }
            }
            if(pp1==NULL && pp2!=NULL)
            {
                delete [] cid1;
                delete [] cid2;
                return 0;
            }
        }
        delete [] cid1;
        delete [] cid2;
        return 1;
    }
}

void PLUS(MarkingMeta &mm1,const MarkingMeta &mm2)
{
    Tokens *p = mm2.tokenQ->next;
    Tokens *resp;
    if(mm1.tid == productsort)
    {
        int sortnum = sorttable.productsort[mm1.sid].sortnum;
        while(p)
        {
            resp = new Tokens;
            Tokenscopy(*resp,*p,mm1.tid,sortnum);
            mm1.insert(resp);
            p=p->next;
        }
    } else{
        while(p)
        {
            resp = new Tokens;
            Tokenscopy(*resp,*p,mm1.tid);
            mm1.insert(resp);
            p=p->next;
        }
    }
}
/***********************************************************************/
index_t MarkingMeta::Hash() {
    index_t hv = 0;
    if(tid == usersort)
    {
        hv = colorcount*H1FACTOR*H1FACTOR*H1FACTOR;
        Tokens *p = tokenQ->next;
        for(p;p!=NULL;p=p->next)
        {
            COLORID cid;
            p->tokens->getColor(cid);
            hv += p->tokencount*H1FACTOR*H1FACTOR+(cid+1)*H1FACTOR;
        }
    }
    else if(tid == productsort)
    {
        hv = colorcount*H1FACTOR*H1FACTOR*H1FACTOR;
        Tokens *p;
        int sortnum = sorttable.productsort[sid].sortnum;
        COLORID *cid = new COLORID[sortnum];
        for(p=tokenQ->next;p!=NULL;p=p->next)
        {
            p->tokens->getColor(cid,sortnum);
            hv += p->tokencount*H1FACTOR*H1FACTOR;
            for(int j=0;j<sortnum;++j)
            {
                hv += (cid[j]+1)*H1FACTOR;
            }
        }
        delete [] cid;
    }
    else if(tid == dot)
    {
        hv = colorcount*H1FACTOR*H1FACTOR*H1FACTOR;
        Tokens *p = tokenQ->next;
        if(p!=NULL)
            hv += p->tokencount*H1FACTOR*H1FACTOR;
    }
    else if(tid == finiteintrange)
    {
        cerr<<"Sorry, we don't support finteintrange now."<<endl;
        exit(-1);
    }
    hashvalue = hv;
    return hv;
}
/*1.找到要插入的位置，分为三种：
 * i.末尾
 * ii.中间
 * iii.开头
 * 2.插入到该插入的位置，并设置colorcount++；
 * (3).如果是相同的color，仅更改tokencount；
 * */
void MarkingMeta::insert(Tokens *t) {
    if(tid == usersort)
    {
        Tokens *q=tokenQ,*p = tokenQ->next;
        COLORID pcid,tcid;
        t->tokens->getColor(tcid);
        while(p!=NULL)
        {
            p->tokens->getColor(pcid);
            if(tcid<=pcid)
                break;
            q=p;
            p=p->next;
        }
        if(p == NULL)
        {
            q->next = t;
            colorcount++;
            return;
        }
        if(tcid == pcid)
        {
            p->tokencount+=t->tokencount;
            delete t;
        }
        else if(tcid<pcid)
        {
            t->next = p;
            q->next = t;
            colorcount++;
        }
    }
    else if(tid == productsort)
    {
        Tokens *q=tokenQ,*p = tokenQ->next;
        COLORID *pcid,*tcid;
        int sortnum = sorttable.productsort[sid].sortnum;
        pcid = new COLORID[sortnum];
        tcid = new COLORID[sortnum];
        t->tokens->getColor(tcid,sortnum);
        while(p!=NULL)
        {
            p->tokens->getColor(pcid,sortnum);
            if(array_lessorequ(tcid,pcid,sortnum))
                break;
            q=p;
            p=p->next;
        }
        if(p == NULL)
        {
            q->next = t;
            colorcount++;
            delete [] pcid;
            delete [] tcid;
            return;
        }
        if(array_equal(tcid,pcid,sortnum))
        {
            p->tokencount += t->tokencount;
            delete t;
        }
        else if(array_less(tcid,pcid,sortnum))
        {
            t->next = p;
            q->next = t;
            colorcount++;
        }
        delete [] pcid;
        delete [] tcid;
    }
    else if(tid == dot)
    {
        if(tokenQ->next == NULL)
            tokenQ->next=t;
        else {
            tokenQ->next->tokencount+=t->tokencount;
            delete t;
        }
        colorcount = 1;
    }
}

bool MarkingMeta::operator>=(const MarkingMeta &mm) {
    bool greaterorequ = true;
    Tokens *p1,*p2;
    p1 = this->tokenQ->next;
    p2 = mm.tokenQ->next;

    if(mm.colorcount==0)
        return true;
    else if(this->colorcount == 0)
        return false;


    if(this->tid == dot)
    {
        if(p1->tokencount>=p2->tokencount)
            greaterorequ = true;
        else
            greaterorequ = false;
    }
    else if(this->tid == usersort)
    {
        COLORID cid1,cid2;
        while(p2)
        {
            //p2中的元素，p1中必须都有；遍历p2
            p2->tokens->getColor(cid2);
            while(p1)
            {
                p1->tokens->getColor(cid1);
                if(cid1<cid2)
                {
                    p1=p1->next;
                    continue;
                }
                else if(cid1==cid2)
                {
                    if(p1->tokencount>=p2->tokencount)
                    {
                        break;
                    }
                    else
                    {
                        greaterorequ = false;
                        break;
                    }
                }
                else if(cid1>cid2)
                {
                    greaterorequ = false;
                    break;
                }
            }
            if(p1 == NULL)
                greaterorequ = false;
            if(!greaterorequ)
                break;
            else{
                p2=p2->next;
            }
        }
    }
    else if(this->tid == productsort)
    {
        int sortnum = sorttable.productsort[sid].sortnum;
        COLORID *cid1,*cid2;
        cid1 = new COLORID[sortnum];
        cid2 = new COLORID[sortnum];
        while(p2)
        {
            p2->tokens->getColor(cid2,sortnum);
            while(p1)
            {
                p1->tokens->getColor(cid1,sortnum);
                if(array_less(cid1,cid2,sortnum)){
                    p1=p1->next;
                    continue;
                }
                else if(array_equal(cid1,cid2,sortnum))
                {
                    if(p1->tokencount>=p2->tokencount)
                    {
                        break;
                    }
                    else
                    {
                        greaterorequ = false;
                        break;
                    }
                }
                else if(array_greater(cid1,cid2,sortnum))
                {
                    greaterorequ = false;
                    break;
                }
            }
            if(p1 == NULL)
                greaterorequ = false;
            if(!greaterorequ)
                break;
            else {
                p2=p2->next;
            }
        }
        delete [] cid1;
        delete [] cid2;
    }
    return greaterorequ;
}

void MarkingMeta::printToken() {
    Tokens *p = tokenQ->next;
    if(p==NULL)
    {
        cout<<"NULL"<<endl;
        return;
    }
    while(p)
    {
        cout<<p->tokencount<<"\'";
        if(tid == usersort)
        {
            COLORID cid;
            p->tokens->getColor(cid);
            cout<<sorttable.usersort[sid].cyclicenumeration[cid];
        }
        else if(tid == productsort)
        {
            int sortnum = sorttable.productsort[sid].sortnum;
            COLORID *cid = new COLORID[sortnum];
            p->tokens->getColor(cid,sortnum);
            cout<<"[";
            for(int i=0;i<sortnum;++i)
            {
                SORTID ssid = sorttable.productsort[sid].sortid[i].sid;
                cout<<sorttable.usersort[ssid].cyclicenumeration[cid[i]];
            }
            cout<<"]";
            delete [] cid;
        }
        else if(tid == dot)
        {
            cout<<"dot";
        }
        p=p->next;
        cout<<'+';
    }
    cout<<endl;
}

NUM_t MarkingMeta::Tokensum() {
    NUM_t sum = 0;
    Tokens *p = tokenQ->next;
    while(p!=NULL)
    {
        sum+=p->tokencount;
        p=p->next;
    }
    return sum;
}

/*****************************************************************/
CPN_RGNode::~CPN_RGNode() {
    delete [] marking;
    //delete firetrans;
    MallocExtension::instance()->ReleaseFreeMemory();
}

index_t CPN_RGNode::Hash(SHORTNUM *weight) {
    index_t hashvalue = 0;
    for(int i=0;i<placecount;++i)
    {
        hashvalue += weight[i]*marking[i].hashvalue;
    }
    return hashvalue;
}

/*function:判断两个状态CPN_RGNode是否为同一个状态；
 * */
bool CPN_RGNode::operator==(const CPN_RGNode &n1) {
    bool equal = true;
    for(int i=0;i<placecount;++i)
    {
        //检查库所i的tokenmetacount是否一样
        SHORTNUM tmc = this->marking[i].colorcount;
        if(tmc != n1.marking[i].colorcount)
        {
            equal = false;
            break;
        }

        //检查每一个tokenmeta是否一样
        Tokens *t1=this->marking[i].tokenQ->next;
        Tokens *t2=n1.marking[i].tokenQ->next;
        for(t1,t2; t1!=NULL && t2!=NULL; t1=t1->next,t2=t2->next)
        {
            //先检查tokencount
            if(t1->tokencount!=t2->tokencount)
            {
                equal = false;
                break;
            }

            //检查color
            type tid = cpnet->place[i].tid;
            if(tid == dot) {
                continue;
            }
            else if(tid == usersort){
                COLORID cid1,cid2;
                t1->tokens->getColor(cid1);
                t2->tokens->getColor(cid2);
                if(cid1!=cid2) {
                    equal = false;
                    break;
                }
            }
            else if(tid == productsort){
                COLORID *cid1,*cid2;
                SORTID sid = cpnet->place[i].sid;
                int sortnum = sorttable.productsort[sid].sortnum;
                cid1 = new COLORID[sortnum];
                cid2 = new COLORID[sortnum];
                t1->tokens->getColor(cid1,sortnum);
                t2->tokens->getColor(cid2,sortnum);
                for(int k=0;k<sortnum;++k)
                {
                    if(cid1[k]!=cid2[k])
                    {
                        equal = false;
                        delete [] cid1;
                        delete [] cid2;
                        break;
                    }
                }
                if(equal==false)
                    break;
                delete [] cid1;
                delete [] cid2;
            }
            else if(tid == finiteintrange){
                cerr<<"[CPN_RG\\line110]FiniteIntRange ERROR.";
                exit(-1);
            }
        }
        if(equal == false)
            break;
    }
    return equal;
}

/*function:复制一个状态
 * */
void CPN_RGNode::operator=(const CPN_RGNode &rgnode) {
    int i;
    for(i=0;i<placecount;++i)
    {
        const MarkingMeta &placemark = rgnode.marking[i];
        MarkingMetacopy(this->marking[i],placemark,placemark.tid,placemark.sid);
    }
}

void CPN_RGNode::printMarking() {
    cout<<"[M"<<numid<<"]"<<endl;
    for(int i=0;i<placecount;++i)
    {
        cout<<setiosflags(ios::left)<<setw(15)<<cpnet->place[i].id<<":";
        this->marking[i].printToken();
    }
    cout<<"----------------------------------"<<endl;
}

void CPN_RGNode::selfcheck() {
    for(int i=0;i<placecount;++i)
    {
        if(marking[i].tid != cpnet->place[i].tid)
        {
            cerr<<"TYPE ERROR!"<<endl;
            exit(-1);
        }
        int cc = 0;
        Tokens *p = marking[i].tokenQ->next;
        while(p)
        {
            cc++;
            p=p->next;
        }
        if(cc!=marking[i].colorcount)
        {
            cerr<<"COLORCOUNT ERROR!"<<endl;
            exit(-1);
        }
        if(marking[i].tid == dot)
        {
            if(cc>1)
            {
                cerr<<"DOT ERROR!"<<endl;
                exit(-1);
            }
        }
    }
}

/*************************TranBindQueue****************************/
void TranBindQueue::insert(Bindind *bound) {
    if(bindQ == NULL) {
        rear = bindQ = bound;
        bound->next = NULL;
        return;
    }
    rear->next = bound;
    bound->next = NULL;
    rear = bound;
}

void TranBindQueue::getSize(int &size) {
    size = 0;
    Bindind *p = bindQ;
    for(p;p!=NULL;p=p->next,++size);
}

void FiTranQueue::insert(TranBindQueue *TBQ) {
    TBQ->next = fireQ;
    fireQ = TBQ;
}

void FiTranQueue::getSize(int &size) {
    size = 0;
    TranBindQueue *p;
    for(p=fireQ;p!=NULL;p=p->next,++size);
}

bool FiTranQueue::isfirable(string transname) const{
    map<string,index_t>::iterator titer;
    titer = cpnet->mapTransition.find(transname);
    unsigned short transid = titer->second;
    TranBindQueue *p = fireQ;
    while(p!=NULL)
    {
        if(p->tranid == transid)
            return true;
        p=p->next;
    }
    return false;
}

/*****************************************************************/
CPN_RG::CPN_RG() {
    initnode = NULL;
    markingtable = new CPN_RGNode*[CPNRGTABLE_SIZE];
    for(int i=0;i<CPNRGTABLE_SIZE;++i) {
        markingtable[i] = NULL;
    }
    nodecount = 0;
    hash_conflict_times = 0;

    weight = new SHORTNUM[placecount];
    srand((int)time(NULL));
    for(int j=0;j<placecount;++j)
    {
        weight[j] = random(11)+1;
    }
}

CPN_RG::~CPN_RG() {
    for(int i=0;i<CPNRGTABLE_SIZE;++i)
    {
        if(markingtable[i]!=NULL)
        {
            CPN_RGNode *p=markingtable[i];
            CPN_RGNode *q;
            while(p)
            {
                q=p->next;
                delete p;
                p=q;
            }
        }
    }
    delete [] markingtable;
    MallocExtension::instance()->ReleaseFreeMemory();
}

/*function：得到一个新的节点后，把这个节点加入到哈希表中；
 * Logics:
 * 1.计算该节点的哈希值；
 * 2.根据哈希值得到索引值，索引值为：hashvalue & size；
 * 3.根据索引值，插入到链式哈希的相应链中；
 * 4.维护nodecount；
 * */
void CPN_RG::addRGNode(CPN_RGNode *mark) {
    mark->numid = nodecount;
    index_t hashvalue = mark->Hash(weight);
    index_t size = CPNRGTABLE_SIZE-1;
    hashvalue = hashvalue & size;

    if(markingtable[hashvalue]!=NULL)
        hash_conflict_times++;

    mark->next = markingtable[hashvalue];
    markingtable[hashvalue] = mark;
    nodecount++;
}

 /*function:得到可达图的初始节点
  * Logics:
  * 1.遍历库所表的每一个库所，对于每一个库所，将这个库所的initMarking链复制到initnode.marking[i]中:
  * 2.遍历initMarking链的每一个Tokens，复制这个Tokens并插入到initnode.marking[i]链中；
  * 3.得到新状态后，不要忘记立马计算每一个库所的哈希值；
  * 4.将初始状态加入到哈西表中
  * */
CPN_RGNode *CPN_RG::CPNRGinitialnode() {
    initnode = new CPN_RGNode;
    //遍历每一个库所
    for(int i=0;i<placecount;++i)
    {
        CPlace &pp = cpnet->place[i];
        MarkingMeta &mm = initnode->marking[i];

        //遍历每一个Tokens
        for(int j=0;j<pp.metacount;++j)
        {
            if(pp.tid!=productsort)
            {
                Tokens *q=new Tokens;
                Tokenscopy(*q,pp.initMarking[j],pp.tid);
                mm.insert(q);
            }
            else
            {
                int sortnum = sorttable.productsort[pp.sid].sortnum;
                Tokens *q=new Tokens;
                Tokenscopy(*q,pp.initMarking[j],pp.tid,sortnum);
                mm.insert(q);
            }
        }
        mm.Hash();
    }
    getFireableTranx(initnode,*(initnode->firetrans));
    addRGNode(initnode);
    return initnode;
}

/*function:根据当前状态，以及当前状态下可发生变迁的一个绑定
 * 计算得到一个新的状态
 * Logics:
 * 1.首相将旧状态复制给新状态
 * 2.可发生变迁的前继库所减去弧上的值（多重集的减法）
 * 3.可发生变迁的后继库所加上弧上的值（要先计算后继弧的多重集，然后做多重集的加法）
 * 4.计算新状态的每一个MarkingMeta的Hash；
 * 5.判断是否为已存在的节点；
 * */
CPN_RGNode *CPN_RG::CPNRGcreatenode(CPN_RGNode *mark,const TranBindQueue *tbq,const Bindind *bindptr,bool &exist) {
    if(bindptr == NULL)
        return NULL;

    CPN_RGNode *newmark = new CPN_RGNode;
    *newmark = *mark;
    CTransition &trans = cpnet->transition[tbq->tranid];
    vector<CSArc>::iterator front;

    //计算前继库所的Tokens
    int i;
    for(i=0,front=trans.producer.begin();front!=trans.producer.end();++front,++i)
    {
        MINUS(newmark->marking[front->idx],bindptr->arcsMultiSet[i]);
    }

    //计算后继库所的Tokens
    vector<CSArc>::iterator rear;
    MarkingMeta *mm = new MarkingMeta[trans.consumer.size()];
    for(i=0,rear=trans.consumer.begin();rear!=trans.consumer.end();++rear,++i)
    {
        mm[i].initiate(cpnet->place[rear->idx].tid,cpnet->place[rear->idx].sid);
        if(cpnet->place[rear->idx].tid == productsort)
        {
            int psnum = 0;
            SORTID sid = cpnet->place[rear->idx].sid;
            psnum = sorttable.productsort[sid].sortnum;
            computeArcEXP(rear->arc_exp,mm[i],bindptr->varvec,psnum);
        }
        else{
            computeArcEXP(rear->arc_exp,mm[i],bindptr->varvec);
        }
        PLUS(newmark->marking[rear->idx],mm[i]);
    }
    delete [] mm;

    for(int i=0;i<placecount;++i)
    {
        newmark->marking[i].Hash();
    }
    CPN_RGNode *existnode =NULL;
    exist = NodeExist(newmark,existnode);
    if(!exist)
    {
        getFireableTranx(newmark,*newmark->firetrans);
        addRGNode(newmark);
//        cout<<"M"<<mark->numid<<"[>"<<trans.id;
//        newmark->printMarking();
//        cout<<"***NODECOUNT:"<<this->nodecount<<"***"<<endl;
        return newmark;
    }
    else
    {
//        cout<<"----------------------------------"<<endl;
//        cout<<"M"<<mark->numid<<"[>"<<trans.id<<" M"<<existnode->numid<<endl;
//        cout<<"----------------------------------"<<endl;
        delete newmark;
        return existnode;
    }
}

bool CPN_RG::NodeExist(CPN_RGNode *mark,CPN_RGNode *&existmark) {
    index_t hashvalue = mark->Hash(weight);
    index_t size = CPNRGTABLE_SIZE-1;
    hashvalue = hashvalue & size;

    bool exist = false;
    CPN_RGNode *p = markingtable[hashvalue];
    while(p)
    {
        if(*p == *mark)
        {
            exist = true;
            existmark = p;
            break;
        }
        p=p->next;
    }
    return exist;
}

/*得到当前状态CPN_RGNode *curnode下所有的可发生变迁，
 * FiTranQueue:可发生变迁队列，FiTranQueue{{TranBindQueue}->{TranBindQueue}}
 * TranBindQueue：可发生变迁，里面还有所有的能使其发生的绑定队列，{{Bindind}->{Bindind}}
 * Bindind:变量的绑定，一个变量向量，如<v1=c1,v2=c2,v3=c3,...>
 * */
void CPN_RG::getFireableTranx(CPN_RGNode *curnode,FiTranQueue &fireableTs){
    //遍历每一个变迁，看其能否发生
    for(int i=transitioncount-1;i>=0;--i)
    {
        TranBindQueue *tbq = new TranBindQueue;
        tbq->tranid = i;

        CTransition &tran = cpnet->transition[i];
        //预先判断
        vector<CSArc>::iterator piter;
        bool possiblefire = true;
        for(piter=tran.producer.begin();piter!=tran.producer.end(); ++piter)
        {
            if(curnode->marking[piter->idx].colorcount == 0)
            {
                possiblefire = false;
                break;
            }
        }

        if(!possiblefire)
            continue;

        int level=0;
        Bindind *bind = new Bindind;
        getTranxBinding(curnode,tran,level,bind,*tbq);
        delete bind;
        int firesize;
        tbq->getSize(firesize);
        if(firesize!=0) {
            fireableTs.insert(tbq);
        }
        else {
            delete tbq;
        }
    }
}


void CPN_RG::getTranxBinding(CPN_RGNode *curnode,const CPN_Transition &tt,int level,Bindind *&bind,TranBindQueue &tbq)
{
//    if(!timeflag)
//        return;
    if(level >= tt.relvararray.size())
    {
        MarkingMeta *mm = new MarkingMeta[tt.producer.size()];
        vector<CSArc>::const_iterator preiter;
        int j;
        bool firebound = true;
        for(j=0,preiter=tt.producer.begin();preiter!=tt.producer.end();++j,++preiter)
        {
            //计算每一个前继弧的多重集，并判断个前继库所的包含关系
            type ptid = cpnet->place[preiter->idx].tid;
            SORTID psid = cpnet->place[preiter->idx].sid;
            mm[j].initiate(ptid,psid);

            int psnum;
            if(ptid == productsort){
                psnum = sorttable.productsort[psid].sortnum;
            }
            else
                psnum = 0;
            computeArcEXP(preiter->arc_exp,mm[j],bind->varvec,psnum);
            if(colorflag==false)
            {
                firebound = false;
                break;
            }

            //判断前继库所中的token和弧上表达式的关系
            if(curnode->marking[preiter->idx] >= mm[j])
                continue;
            else
            {
                firebound = false;
                break;
            }
        }

        if(firebound)
        {
            //判断guard函数
            if(tt.hasguard)
            {
                judgeGuard(tt.guard.root->left,bind->varvec);
                firebound = tt.guard.root->left->mytruth;
            }
            if(firebound)
            {
                bind->arcsMultiSet = mm;
                tbq.insert(bind);
                Bindind *pp = new Bindind;
                memcpy(pp->varvec,bind->varvec, sizeof(COLORID)*varcount);
                bind = pp;
            }
        }
        else
            delete [] mm;
        return;
    }

    COLORID i=0;
    SORTID sid = tt.relvararray[level].sid;
    COLORID end = sorttable.usersort[sid].feconstnum;
    for(i=0;i<end;++i)
    {
        bind->varvec[tt.relvararray[level].vid] = i;
        getTranxBinding(curnode,tt,level+1,bind,tbq);
    }
}

void CPN_RG::computeArcEXP(const arc_expression &arcexp,MarkingMeta &mm,COLORID *varcolors,int psnum) {
    colorflag = true;
    computeArcEXP(arcexp.root->leftnode,mm,varcolors,psnum);
}

/*function:将未绑定过的抽象语法树转化为MarkingMeta
 * */
void CPN_RG::computeArcEXP(meta *expnode,MarkingMeta &mm,COLORID *varcolors,int psnum) {
    if(colorflag == false)
        return;
    if(expnode->myname == "add")
    {
        computeArcEXP(expnode->leftnode,mm,varcolors,psnum);
        if(expnode->rightnode!=NULL)
            computeArcEXP(expnode->rightnode,mm,varcolors,psnum);
    }
    else if(expnode->myname == "subtract")
    {
        MarkingMeta mm1,mm2;
        mm1.initiate(mm.tid,mm.sid);
        mm2.initiate(mm.tid,mm.sid);
        computeArcEXP(expnode->leftnode,mm1,varcolors,psnum);
        computeArcEXP(expnode->rightnode,mm2,varcolors,psnum);
        if(MINUS(mm1,mm2)==0)
        {
            colorflag = false;
            return;
        }
        else
        {
            MarkingMetacopy(mm,mm1,mm.tid,mm.sid);
        }
    }
    else if(expnode->myname == "numberof")
    {
        /*numberof节点等同于多重集合中的一个元，如1'a+2'b中的1'a
         * 处理分两步：
         * 1.取出左边节点的数,如1'a中的1;
         * 2.取出有边节点的color，如1'a中的a；
         * */

        //doing the first job;
        SHORTNUM num = expnode->leftnode->number;
        //doing the second job;
        meta *color = expnode->rightnode;
        if(color->myname == "tuple")
        {
            //tuple取出来的是一个数组cid
            COLORID *cid = new COLORID[psnum];
            getTupleColor(color,cid,varcolors,0);
            //创建一个colortoken，插入到mm中
            Tokens *t = new Tokens;
            t->initiate(num,productsort,psnum);
            t->tokens->setColor(cid,psnum);
            mm.insert(t);
            delete [] cid;
        }
        else if(color->myname == "all")
        {
            meta *sortname = color->leftnode;
            map<string,MSI>::iterator siter;
            siter = sorttable.mapSort.find(sortname->myname);
            SHORTNUM feconstnum = sorttable.usersort[siter->second.sid].feconstnum;
            for(COLORID i=0;i<feconstnum;++i)
            {
                Tokens *p = new Tokens;
                p->initiate(num,usersort);
                p->tokens->setColor(i);
                mm.insert(p);
            }
        }
        else if(color->myname == "successor")
        {
            SHORTNUM feconstnum;
            if(color->leftnode->mytype == var)
            {
                map<string,VARID>::iterator viter;
                viter = cpnet->mapVariable.find(color->leftnode->myname);
                SORTID sid = cpnet->vartable[viter->second].sid;
                feconstnum = sorttable.usersort[sid].feconstnum;

                COLORID cid = (varcolors[viter->second]+1)%feconstnum;
                Tokens *p = new Tokens;
                p->initiate(num,usersort);
                p->tokens->setColor(cid);
                mm.insert(p);
            }
            else
            {
                cerr<<"ERROR!CPN_RG::computeArcEXP"<<endl;
            }
        }
        else if(color->myname == "predecessor")
        {
            SHORTNUM feconstnum;
            if(color->leftnode->mytype == var)
            {
                map<string,VARID>::iterator viter;
                viter = cpnet->mapVariable.find(color->leftnode->myname);
                SORTID sid = cpnet->vartable[viter->second].sid;
                feconstnum = sorttable.usersort[sid].feconstnum;

                COLORID cid;
                if(varcolors[viter->second] == 0)
                    cid = feconstnum-1;
                else
                    cid = varcolors[viter->second]-1;
                Tokens *p = new Tokens;
                p->initiate(num,usersort);
                p->tokens->setColor(cid);
                mm.insert(p);
            }
            else
            {
                cerr<<"ERROR!CPN_RG::computeArcEXP"<<endl;
            }
        }
        else if(color->mytype == delsort)
        {
            Tokens *p = new Tokens;
            if(color->myname == "dotconstant")
            {
                p->initiate(num,dot);
                mm.insert(p);
                return;
            }

            COLORID cid;
            map<string,MCI>::iterator citer;
            citer = sorttable.mapColor.find(color->myname);
            cid = citer->second.cid;

            p->initiate(num,usersort);
            p->tokens->setColor(cid);
            mm.insert(p);
        }
        else if(color->mytype == var)
        {
            COLORID cid;
            map<string,VARID>::iterator viter;
            viter = cpnet->mapVariable.find(color->myname);
            cid = varcolors[viter->second];
            Tokens *p = new Tokens;
            p->initiate(num,usersort);
            p->tokens->setColor(cid);
            mm.insert(p);
        }
        else
            cerr<<"ERROR!CPN_RG::computeArcEXP @ line 1186"<<endl;
    }
    else{
        cerr<<"[CPN_RG::computeArcEXP] ERROR:Unexpected arc_expression node"<<expnode->myname<<endl;
        exit(-1);
    }
}

/*function：得到一个未绑定过的语法树中ProductSort类型的颜色（元祖）
 * */
void CPN_RG::getTupleColor(meta *expnode,COLORID *cid,COLORID *varcolors,int ptr) {
    if(expnode->mytype == var)
    {
        //1.首先检查该变量是否已经绑定
        //2.取出color，放在数组cid[ptr]；
        map<string,VARID>::iterator viter;
        viter = cpnet->mapVariable.find(expnode->myname);
        cid[ptr] = varcolors[viter->second];
    }
    else if(expnode->mytype == delsort)
    {
        //根据颜色的名字索引颜色的索引值
        map<string,MCI>::iterator citer;
        citer = sorttable.mapColor.find(expnode->myname);
        cid[ptr] = citer->second.cid;
    }
    else if(expnode->myname == "tuple")
    {
        getTupleColor(expnode->leftnode,cid,varcolors,ptr);
        getTupleColor(expnode->rightnode,cid,varcolors,ptr+1);
    }
    else if(expnode->myname == "successor")
    {
        SHORTNUM feconstnum;
        if(expnode->leftnode->mytype == var)
        {
            map<string,VARID>::iterator viter;
            viter = cpnet->mapVariable.find(expnode->leftnode->myname);  //根据变量找到变量的索引值
            SORTID sid = cpnet->vartable[viter->second].sid;
            feconstnum = sorttable.usersort[sid].feconstnum;

            cid[ptr] = (varcolors[viter->second]+1)%feconstnum;
        }
        else if(expnode->leftnode->mytype == delsort)
        {
            map<string,MCI>::iterator citer;
            citer = sorttable.mapColor.find(expnode->myname);
            feconstnum = sorttable.usersort[citer->second.sid].feconstnum;

            cid[ptr] = (citer->second.cid+1)%feconstnum;
        }
    }
    else if(expnode->myname == "predecessor")
    {
        SHORTNUM feconstnum;
        if(expnode->leftnode->mytype == var)
        {
            map<string,VARID>::iterator viter;
            viter = cpnet->mapVariable.find(expnode->leftnode->myname);
            SORTID sid = cpnet->vartable[viter->second].sid;
            feconstnum = sorttable.usersort[sid].feconstnum;

            COLORID ccc = varcolors[viter->second];

            if(ccc == MAXSHORT)
            {
                cerr<<"[CPN_RG::getTupleColor]ERROR：variable "<<expnode->leftnode->myname<<" is not bounded."<<endl;
                exit(-1);
            }

            if(ccc == 0)
                cid[ptr] = feconstnum-1;
            else
                cid[ptr] = ccc-1;
        }
        else if(expnode->leftnode->mytype == delsort)
        {
            map<string,MCI>::iterator citer;
            citer = sorttable.mapColor.find(expnode->myname);  //根据该变量名字找到该变量
            feconstnum = sorttable.usersort[citer->second.sid].feconstnum;

            if(citer->second.cid == 0)
                cid[ptr] = feconstnum-1;
            else
                cid[ptr] = citer->second.cid-1;
        }
    }
}

/*function：判断一个变迁的Guard函数（抽象语法树）是否为真
 * */
void CPN_RG::judgeGuard(CTN *node,COLORID *cid) {
    switch(node->mytype)
    {
        case Boolean:{
            if(node->myname == "and")
            {
                judgeGuard(node->left,cid);
                judgeGuard(node->right,cid);
                if(!node->left->mytruth)
                    node->mytruth = false;
                else
                    node->mytruth = node->right->mytruth;
            }
            else if(node->myname == "or")
            {
                judgeGuard(node->left,cid);
                judgeGuard(node->right,cid);
                if(node->left->mytruth)
                    node->mytruth = true;
                else
                    node->mytruth = node->right->mytruth;
            }
            else if(node->myname == "imply")
            {
                judgeGuard(node->left,cid);
                judgeGuard(node->right,cid);
                if(!node->left->mytruth)
                    node->mytruth = true;
                else if(node->left->mytruth && node->right->mytruth)
                    node->mytruth = true;
                else
                    node->mytruth = false;
            }
            else if(node->myname == "not")
            {
                judgeGuard(node->left,cid);
                if(node->left->mytruth)
                    node->mytruth = false;
                else
                    node->mytruth = true;
            }
            break;
        }
        case Relation:{
            if(node->left->mytype!=variable && node->left->mytype!=useroperator)
                cerr<<"[condition_tree::judgeGuard]ERROR:Relation node's leftnode is not a variable or color."<<endl;
            if(node->right->mytype!=variable && node->right->mytype!=useroperator)
                cerr<<"[condition_tree::judgeGuard]ERROR:Relation node's rightnode is not a variable or color."<<endl;
            judgeGuard(node->left,cid);
            judgeGuard(node->right,cid);
            if(node->myname == "equality")
            {
                if(node->left->cid == node->right->cid)
                    node->mytruth = true;
                else
                    node->mytruth = false;
            }
            else if(node->myname == "inequality")
            {
                if(node->left->cid != node->right->cid)
                    node->mytruth = true;
                else
                    node->mytruth = false;
            }
            else if(node->myname == "lessthan")
            {
                if(node->left->cid < node->right->cid)
                    node->mytruth = true;
                else
                    node->mytruth = false;
            }
            else if(node->myname == "lessthanorequal")
            {
                if(node->left->cid <= node->right->cid)
                    node->mytruth = true;
                else
                    node->mytruth = false;
            }
            else if(node->myname == "greaterthan")
            {
                if(node->left->cid > node->right->cid)
                    node->mytruth = true;
                else
                    node->mytruth = false;
            }
            else if(node->myname == "greaterthanorequal")
            {
                if(node->left->cid >= node->right->cid)
                    node->mytruth = true;
                else
                    node->mytruth = false;
            }
            break;
        }
        case variable:{
            map<string,VARID>::iterator viter;
            viter=cpnet->mapVariable.find(node->myname);
            if(cid[viter->second] == MAXSHORT)
            {
                cerr<<"[condition_tree::judgeGuard]ERROR:Variable "<<node->myname<<" is not bounded."<<endl;
                exit(-1);
            }
            node->cid = cid[viter->second];
            break;
        }
        case useroperator: {
            map<string,mapcolor_info>::iterator citer;
            citer = sorttable.mapColor.find(node->myname);
            node->cid = citer->second.cid;
            break;
        }
        default:{
            cerr<<"[judgeGuard] Unrecognized node in condition tree"<<endl;
            exit(-1);
        }
    }
}

/*function:生成完整的可达图
 * Logics:
 * 1.计算当前状态下的所有可发生变迁（每个变迁的每种可发生绑定）
 * 2.两层循环：对于所有的可发生变迁，以及该变迁下的每一种可发生绑定
 * 3.计算新的状态，并判断该状态是否为重复状态
 * 4.若不是重复状态，则加入到哈希表中。
 * */
void CPN_RG::Generate(CPN_RGNode *mark) {
    TranBindQueue *fireptr = mark->firetrans->fireQ;
    Bindind *bindptr;

    int firesize;
    mark->firetrans->getSize(firesize);
    while(fireptr)
    {
        bindptr = fireptr->bindQ;
        int bindsize;
        fireptr->getSize(bindsize);
        bool exist;
        while(bindptr)
        {
            CPN_RGNode *nextnode = CPNRGcreatenode(mark,fireptr,bindptr,exist);
            if(!exist)
            {
                Generate(nextnode);
            }
            bindptr = bindptr->next;
        }
        fireptr = fireptr->next;
    }
}

