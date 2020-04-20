//
// Created by hecong on 19-11-23.
//
#include "CPN.h"
SortTable sorttable;

int stringToNum(const string& str)
{
    istringstream iss(str);
    int num;
    iss >> num;
    return num;
}

void CPN_Place::printTokens(string &str)
{
    if(tid == usersort)
    {
        for(int i=0;i<metacount;++i)
        {
            COLORID cid;
            initMarking[i].tokens->getColor(cid);
            str += to_string(initMarking[i].tokencount)+"\'["+sorttable.usersort[sid].cyclicenumeration[cid]+"] ";
        }

    }
    else if(tid == productsort)
    {
        for(int i=0;i<metacount;++i)
        {
            int membernum = sorttable.productsort[sid].sortnum;
            COLORID *cid = new COLORID[membernum];
            initMarking[i].tokens->getColor(cid,membernum);
            str += to_string(initMarking[i].tokencount)+"\'[";
            for(int j=0;j<membernum;++j)
            {
                MSI m=sorttable.productsort[sid].sortid[j];
                str+=sorttable.usersort[m.sid].cyclicenumeration[cid[j]]+" ";
            }
            str+= "] ";
            delete cid;
        }
    }
    else if(tid == finiteintrange)
    {
        cerr<<"[CPN.h\\line227]Sorry,we don't support this sort now."<<endl;
        exit(-1);
    }
    else if(tid == dot)
    {
        for(int i=0;i<metacount;++i)
        {
            str+=to_string(initMarking[i].tokencount)+"\'[dot] ";
        }
    }
}

SORTID SortTable::getFIRid(string FIRname) {
    vector<FiniteIntRange>::iterator iter;
    unsigned short idx = 0;
    for(iter=finitintrange.begin();iter!=finitintrange.end();++iter,++idx)
    {
        if(iter->id == FIRname)
            return idx;
    }
    return MAXSHORT;
}

SORTID SortTable::getPSid(string PSname) {
    vector<ProductSort>::iterator iter;
    unsigned short idx = 0;
    for(iter=productsort.begin();iter!=productsort.end();++iter,++idx)
    {
        if(iter->id == PSname)
            return idx;
    }
    return MAXSHORT;
}

SORTID SortTable::getUSid(string USname) {
    vector<UserSort>::iterator iter;
    unsigned short idx = 0;
    for(iter=usersort.begin();iter!=usersort.end();++iter,++idx)
    {
        if(iter->id == USname)
            return idx;
    }
    return MAXSHORT;
}

void Tokens::initiate(SHORTNUM tc,type sort,int PSnum) {
    tokencount = tc;
    switch(sort)
    {
        case type::productsort:{
            if(PSnum!=0)
                tokens = new ProductSortValue(PSnum);
            break;
        }
        case type::usersort:{
            tokens = new UserSortValue;
            break;
        }
        case type::dot:{
            tokens = NULL;
            break;
        }
        case type::finiteintrange:{
            tokens = new FIRSortValue;
            break;
        }
        default:{
            cerr<<"Sorry! Can not support this sort."<<endl;
            exit(-1);
        }
    }
}

CPN::CPN() {
    placecount = 0;
    transitioncount = 0;
    arccount = 0;
    varcount = 0;
}
/*This function is to first scan the pnml file and do these things:
 * 1. get the number of places,transitions,arcs and variables;
 * 2. alloc space for place table, transition table, arc table,
 * P_sorttable and variable table;
 * 3. parse declarations, get all sorts;
 * */
void CPN::getSize(char *filename) {
    TiXmlDocument *mydoc = new TiXmlDocument(filename);
    if(!mydoc->LoadFile()) {
        cerr<<mydoc->ErrorDesc()<<endl;
    }

    TiXmlElement *root = mydoc->RootElement();
    if(root == NULL) {
        cerr<<"Failed to load file: no root element!"<<endl;
        mydoc->Clear();
    }

    TiXmlElement *net = root->FirstChildElement("net");
    TiXmlElement *page = net->FirstChildElement("page");

    //doing the first job;
    while(page)
    {
        TiXmlElement *pageElement = page->FirstChildElement();
        while(pageElement)
        {
            string value = pageElement->Value();
            if(value == "place") {
                ++placecount;
            }
            else if(value == "transition") {
                transitioncount++;
            }
            else if(value == "arc") {
                arccount++;
            }
            pageElement = pageElement->NextSiblingElement();
        }
        page = page->NextSiblingElement();
    }

    //doing the second job
    place = new CPlace[placecount];
    transition = new CTransition[transitioncount];
    arc = new CArc[arccount];

    //doing the third job
    TiXmlElement *decl = net->FirstChildElement("declaration");
    TiXmlElement *decls = decl->FirstChildElement()->FirstChildElement("declarations");
    TiXmlElement *declContent = decls->FirstChildElement();
    SORTID psptr=0,usptr=0,firptr=0;
    while(declContent)
    {
        string value = declContent->Value();
        if(value == "namedsort")
        {
            TiXmlElement *sort = declContent->FirstChildElement();
            string sortname = sort->Value();
            if(sortname == "cyclicenumeration")
            {
                UserSort uu;
                uu.mytype = usersort;
                uu.id = declContent->FirstAttribute()->Value();
                TiXmlElement *feconstant = sort->FirstChildElement();
                COLORID colorindex;
                for(uu.feconstnum=0,colorindex=0; feconstant; ++uu.feconstnum,++colorindex,feconstant=feconstant->NextSiblingElement())
                {
                    string feconstid = feconstant->FirstAttribute()->Value();
                    uu.cyclicenumeration.push_back(feconstid);
                    uu.mapValue.insert(pair<string,SORTID>(feconstid,colorindex));
                    MCI  mci;
                    mci.cid = colorindex;
                    mci.sid = usptr;
                    sorttable.mapColor.insert(pair<string,MCI>(feconstid,mci));
                }
                sorttable.usersort.push_back(uu);

                MSI msi;
                msi.tid = usersort;
                msi.sid = usptr++;
                sorttable.mapSort.insert(pair<string,MSI>(uu.id,msi));
            }
            else if(sortname == "productsort")
            {
                ProductSort pp;
                pp.id = declContent->FirstAttribute()->Value();
                pp.mytype = productsort;
                int sortcount = 0;
                TiXmlElement *usersort = sort->FirstChildElement();
                for(sortcount=0;usersort;++sortcount,usersort=usersort->NextSiblingElement())
                {
                    string ustname = usersort->Value();
                    if(ustname != "usersort") {
                        cerr<<"Unexpected sort \'"<<ustname<<"\'. We assumed that every sort of productsort is usersort."<<endl;
                        exit(-1);
                    }
                    string relatedsortname = usersort->FirstAttribute()->Value();
                    pp.sortname.push_back(relatedsortname);
                }
                pp.sortnum = sortcount;
                sorttable.productsort.push_back(pp);

                MSI m;
                m.sid = psptr++;
                m.tid = productsort;
                sorttable.mapSort.insert(pair<string,MSI>(pp.id,m));
            }
            else if(sortname == "finiteintrange")
            {
                cerr<<"Find you, finiteintrange!"<<endl;
                exit(-1);
            }
            else if(sortname == "dot") {
                sorttable.hasdot = true;
            }
        }
        else if(value == "variabledecl")
        {
            ++varcount;
        }
        else
        {
            cerr<<"Can not support XmlElement "<<value<<endl;
            exit(-1);
        }
        declContent = declContent->NextSiblingElement();
    }

    vector<ProductSort>::iterator psiter;
    for(psiter=sorttable.productsort.begin();psiter!=sorttable.productsort.end();++psiter)
    {
        vector<string>::iterator miter;
        for(miter=psiter->sortname.begin();miter!=psiter->sortname.end();++miter)
        {
            map<string,MSI>::iterator citer;
            citer = sorttable.mapSort.find(*miter);
            if(citer!=sorttable.mapSort.end())
            {
                psiter->sortid.push_back(citer->second);
            }
            else cerr<<"[CPN.cpp\\line223].ProductSort error."<<endl;
        }
    }
    vartable = new Variable[varcount];
}

/*This function is to second scan pnml file just after getSize();
 * It does these jobs:
 * 1.parse pnml file, get every place's information,especially initMarking,
 * get every transition's information,especially guard fucntion;
 * get every arc's information, especially arc expressiion
 * 2.get all variable's information, set up variable table;
 * 3.construct the CPN, complete every place and transition's producer and consumer
 * */
void CPN::readPNML(char *filename) {
    TiXmlDocument *mydoc = new TiXmlDocument(filename);
    if(!mydoc->LoadFile()) {
        cerr<<mydoc->ErrorDesc()<<endl;
    }

    TiXmlElement *root = mydoc->RootElement();
    if(root == NULL) {
        cerr<<"Failed to load file: no root element!"<<endl;
        mydoc->Clear();
    }

    TiXmlElement *net = root->FirstChildElement("net");
    TiXmlElement *page = net->FirstChildElement("page");

    index_t pptr = 0;
    index_t tptr = 0;
    index_t aptr = 0;
    index_t vpter = 0;
    while(page)
    {
        TiXmlElement *pageElement = page->FirstChildElement();
        for(;pageElement;pageElement=pageElement->NextSiblingElement())
        {
            string value = pageElement->Value();
            if(value == "place"){
                CPlace &pp = place[pptr];
                //get id
                TiXmlAttribute *attr = pageElement->FirstAttribute();
                pp.id = attr->Value();
                //get type(mysort,mysortid)
                TiXmlElement *ttype = pageElement->FirstChildElement("type")
                        ->FirstChildElement("structure")->FirstChildElement("usersort");
                string sortname = ttype->FirstAttribute()->Value();
                if(sortname == "dot")
                {
                    pp.tid=dot;
                }
                else{
                    map<string,MSI>::iterator sortiter;
                    sortiter = sorttable.mapSort.find(sortname);
                    if(sortiter!=sorttable.mapSort.end()) {
                        pp.sid = (sortiter->second).sid;
                        pp.tid = (sortiter->second).tid;
                    }
                    else
                    {
                        cerr<<"Can not recognize sort \'"<<sortname<<"\'."<<endl;
                        exit(-1);
                    }
                }
                //get initMarking

                TiXmlElement *initM = pageElement->FirstChildElement("hlinitialMarking");
                if(initM!=NULL){
                    TiXmlElement *firstoperator = initM->FirstChildElement("structure")->FirstChildElement();
                    string opstr = firstoperator->Value();
                    if(opstr == "all")
                    {
                        if(pp.tid!=usersort){
                            cerr<<"ERROR @ line340. The \'all\' operator is not used for cyclicenumeration"<<endl;
                            exit(-1);
                        }
                        UserSort &uu = sorttable.usersort[pp.sid];
                        pp.initMarking = new Tokens[uu.feconstnum];
                        pp.metacount = uu.feconstnum;
                        COLORID i;
                        for(i=0;i<uu.feconstnum;++i)
                        {
                            Tokens &t = pp.initMarking[i];
                            t.initiate(1,usersort);
                            t.tokens->setColor(i);
                        }
                    }
                    else{
                        getInitMarking(firstoperator,pp,0);
                    }
                }
                mapPlace.insert(pair<string,index_t>(pp.id,pptr));
                ++pptr;
            }
            else if(value == "transition")
            {
                CTransition &tt = transition[tptr];
                tt.id = pageElement->FirstAttribute()->Value();
                TiXmlElement *condition = pageElement->FirstChildElement("condition");
                if(condition)
                {
                    tt.guard.constructor(condition);
                    tt.hasguard = true;
                }
                else
                {
                    tt.hasguard = false;
                }
                mapTransition.insert(pair<string,index_t>(tt.id,tptr));
                ++tptr;
            }
            else if(value == "arc")
            {
                CArc &aa = arc[aptr];
                TiXmlAttribute *attr = pageElement->FirstAttribute();
                aa.id = attr->Value();
                attr = attr->Next();
                aa.source_id = attr->Value();
                attr = attr->Next();
                aa.target_id = attr->Value();
                TiXmlElement *hlinscription = pageElement->FirstChildElement("hlinscription");
                aa.arc_exp.constructor(hlinscription);
                ++aptr;
            }
        }
        page = page->NextSiblingElement();
    }

    //doing the second job:
    TiXmlElement *declContent = net->FirstChildElement("declaration")
                                    ->FirstChildElement("structure")
                                    ->FirstChildElement("declarations")
                                    ->FirstChildElement();
    while(declContent)
    {
        string value = declContent->Value();
        if(value == "variabledecl")
        {
            vartable[vpter].id = declContent->FirstAttribute()->Value();
            vartable[vpter].vid = vpter;
            TiXmlElement *ust = declContent->FirstChildElement();
            string ustname = ust->Value();
            if(ustname != "usersort")
            {
                cerr<<"Unexpected sort\'"<<ustname<<"\'. We assumed that every member of productsort is usersort."<<endl;
                exit(-1);
            }
            string sortname = ust->FirstAttribute()->Value();
            map<string,MSI>::iterator iter;
            iter = sorttable.mapSort.find(sortname);
            vartable[vpter].sid = (iter->second).sid;
            vartable[vpter].tid = (iter->second).tid;
            mapVariable.insert(pair<string,VARID>(vartable[vpter].id,vpter));
            if((iter->second).tid != usersort)
            {
                cerr<<"Sorry, but by now, we only support variables whose type is \'cyclicenumeration\'."<<endl;
                exit(-1);
            }
            ++vpter;
        }
        declContent = declContent->NextSiblingElement();
    }

    //doing the third job
    for(int i=0;i<arccount;++i)
    {
        CArc &aa = arc[i];
        map<string,index_t>::iterator souiter;
        map<string,index_t>::iterator tagiter;
        souiter = mapPlace.find(aa.source_id);
        if(souiter == mapPlace.end())
        {
            //souiter是一个变迁
            aa.isp2t = false;
            souiter = mapTransition.find(aa.source_id);
            tagiter = mapPlace.find(aa.target_id);
            CSArc forplace,fortrans;

            forplace.idx = souiter->second;
            forplace.arc_exp = aa.arc_exp;
            place[tagiter->second].producer.push_back(forplace);

            fortrans.idx = tagiter->second;
            fortrans.arc_exp = aa.arc_exp;
            transition[souiter->second].consumer.push_back(fortrans);
        }
        else
        {
            aa.isp2t = true;
            tagiter = mapTransition.find(aa.target_id);
            CSArc forplace,fortrans;

            forplace.idx = tagiter->second;
            forplace.arc_exp = aa.arc_exp;
            place[souiter->second].consumer.push_back(forplace);

            fortrans.idx = souiter->second;
            fortrans.arc_exp = aa.arc_exp;
            transition[tagiter->second].producer.push_back(fortrans);
        }
    }
}

void CPN::getInitMarking(TiXmlElement *elem,CPlace &pp,int i) {
    string value = elem->Value();
    if(value == "subterm")
    {
        elem = elem->FirstChildElement();
        value = elem->Value();
    }

    if(value == "add")
    {
        SHORTNUM metanum = 0;
        TiXmlElement *term = elem->FirstChildElement();
        while(term)
        {
            ++metanum;
            term = term->NextSiblingElement();
        }
        pp.initMarking = new Tokens[metanum];
        pp.metacount = metanum;
        term = elem->FirstChildElement();
        if(i!=0)
            cerr<<"ERROR@CPN.cpp\\line406"<<endl;
        for(int j=0;term;++j,term = term->NextSiblingElement())
        {
            getInitMarking(term,pp,j);
        }
    }
    else if(value == "numberof")
    {
        TiXmlElement *leftchild,*rightchild;
        leftchild = elem->FirstChildElement();
        rightchild = leftchild->NextSiblingElement();

        //leftchild gets number
        string numstr = leftchild->FirstChildElement("numberconstant")
                                ->FirstAttribute()->Value();
        SHORTNUM num = stringToNum(numstr);

        //rightchild get color
        TiXmlElement *st = rightchild->FirstChildElement();
        string stvalue = st->Value();

        if(pp.tid == dot)
        {
            if(pp.initMarking == NULL)
            {
                pp.initMarking= new Tokens[1];
                pp.metacount = 1;
                pp.initMarking->initiate(num,pp.tid);
            }
            else
                pp.initMarking[i].initiate(num,pp.tid);
        }
        else if(pp.tid == productsort)
        {
            Tokens *t;
            if(pp.initMarking == NULL)
            {
                pp.initMarking= new Tokens[1];
                pp.metacount = 1;
                t = pp.initMarking;
            }
            else{
                t = &(pp.initMarking[i]);
            }
            t->initiate(num,pp.tid,sorttable.productsort[pp.sid].sortnum);
            if(stvalue == "tuple")
            {
                ProductSort &ps = sorttable.productsort[pp.sid];
                COLORID *cid = new COLORID[ps.sortnum];
                TiXmlElement *term = st->FirstChildElement();
                for(int i=0; term; ++i,term = term->NextSiblingElement())
                {
                    TiXmlElement *ust = term->FirstChildElement("useroperator");
                    string colorstr = ust->FirstAttribute()->Value();
                    map<string,MCI>::iterator coliter;
                    coliter = sorttable.mapColor.find(colorstr);
                    cid[i] = coliter->second.cid;
                }
                t->tokens->setColor(cid,ps.sortnum);
            }
            else cerr<<"Marking's type doesn't match the place type."<<endl;
        }
        else if(pp.tid == usersort)
        {
            if(stvalue == "useroperator")
            {
                Tokens *t;
                if(pp.initMarking == NULL)
                {
                    pp.initMarking= new Tokens[1];
                    pp.metacount = 1;
                    t = pp.initMarking;
                }
                else
                {
                    t = &(pp.initMarking[i]);
                }
                t->initiate(num,pp.tid);
                string colorstr = st->FirstAttribute()->Value();
                map<string,MCI>::iterator coliter;
                coliter = sorttable.mapColor.find(colorstr);
                t->tokens->setColor(coliter->second.cid);
            }
            else if(stvalue == "all")
            {
                TiXmlElement *ust = st->FirstChildElement("usersort");
                if(pp.tid != usersort) {
                    cerr<<"We assume that operator 'all' is only used for cyclicenumeration type."<<endl;
                    exit(-1);
                }

                //get the sort;
                UserSort &uu = sorttable.usersort[pp.sid];
                if(pp.initMarking!=NULL)
                    cerr<<"ERROR@CPN.cpp\\line486"<<endl;
                pp.initMarking = new Tokens[uu.feconstnum];
                pp.metacount = uu.feconstnum;
                //create tokens;
                COLORID i;
                for(i=0;i<uu.feconstnum;++i)
                {
                    Tokens &t = pp.initMarking[i];
                    t.initiate(num,usersort);
                    t.tokens->setColor(i);
                }
            }
        }
    }
}

CPN::~CPN() {
    delete [] place;
    delete [] transition;
    delete [] arc;
    delete [] vartable;
}

void CPN::printCPN() {
    char placefile[]="place_info.txt";
//    char transfile[]="transition_info.txt";
//    char arcfile[]="arc_info.txt";
    ofstream outplace(placefile,ios::out);
//    ofstream outtrans(transfile,ios::out);
//    ofstream outarc(arcfile,ios::out);

    outplace<<"PLACE INFO {"<<endl;
    for(int i=0;i<placecount;++i)
    {
        string marking;
        place[i].printTokens(marking);
        outplace<<"\t"<<place[i].id<<" "<<marking<<endl;
    }
    outplace<<"}"<<endl;

    /********************print dot graph*********************/
    char dotfile[]="CPN.dot";
    ofstream outdot(dotfile,ios::out);
    outdot<<"digraph CPN {"<<endl;
    for(int i=0;i<placecount;++i)
    {
        outdot<<"\t"<<place[i].id<<" [shape=circle]"<<endl;
    }
    outdot<<endl;
    for(int i=0;i<transitioncount;++i)
    {
        CTransition &t = transition[i];
        if(t.hasguard)
        {
            string guardexp;
            t.guard.printEXP(guardexp);
            outdot<<"\t"<<transition[i].id<<" [shape=box,label=\""<<transition[i].id<<"//"<<guardexp<<"\"]"<<endl;
        }
        else outdot<<"\t"<<transition[i].id<<" [shape=box]"<<endl;
    }
    for(int i=0;i<arccount;++i)
    {
        CArc &a = arc[i];
        string arcexp;
        a.arc_exp.printEXP(arcexp);
        outdot<<"\t"<<a.source_id<<"->"<<a.target_id<<" [label=\""<<arcexp<<"\"]"<<endl;
    }
    outdot<<"}"<<endl;
}

void CPN::printSort() {
    char sortfile[]="sorts_info.txt";
    ofstream outsort(sortfile,ios::out);

    outsort<<"USERSORT:"<<endl;
    vector<UserSort>::iterator uiter;
    for(uiter=sorttable.usersort.begin();uiter!=sorttable.usersort.end();++uiter)
    {
        outsort<<"name: "<<uiter->id<<endl;
        outsort<<"constants: (";
        for(int i=0;i<uiter->feconstnum;++i)
        {
            outsort<<uiter->cyclicenumeration[i]<<",";
        }
        outsort<<")"<<endl;
        outsort<<"------------------------"<<endl;
    }
    outsort<<endl;

    outsort<<"PRODUCTSORT:"<<endl;
    vector<ProductSort>::iterator psiter;
    for(psiter=sorttable.productsort.begin();psiter!=sorttable.productsort.end();++psiter)
    {
        outsort<<"name: "<<psiter->id<<endl;
        outsort<<"member: (";
        for(int i=0;i<psiter->sortnum;++i)
        {
            outsort<<psiter->sortname[i]<<",";
        }
        outsort<<")"<<endl;
        outsort<<"------------------------"<<endl;
    }
    outsort<<endl;
}

void CPN::printVar() {
    ofstream outvar("sorts_info.txt",ios::app);
    outvar<<"VARIABLES:"<<endl;
    for(int i=0;i<varcount;++i)
    {
        outvar<<"name: "<<vartable[i].id<<endl;
        outvar<<"related sort: "<<sorttable.usersort[vartable[i].sid].id<<endl;
        outvar<<"------------------------"<<endl;
    }
}

void CPN::setGlobalVar() {
    ::placecount = this->placecount;
    ::transitioncount = this->transitioncount;
    ::varcount = this->varcount;
}

void CPN::getRelVars() {
    for(int i=0;i<transitioncount;++i)
    {
        CPN_Transition &tt = transition[i];
        vector<CSArc>::iterator preiter;
        //遍历所有前继弧的抽象语法树，得到所有变量
        for(preiter=tt.producer.begin();preiter!=tt.producer.end();++preiter)
        {
            TraArcTreeforVAR(preiter->arc_exp.root,tt.relvars);
        }

        vector<CSArc>::iterator postiter;
        for(postiter=tt.consumer.begin();postiter!=tt.consumer.end();++postiter)
        {
            TraArcTreeforVAR(postiter->arc_exp.root,tt.relvars);
        }

        set<Variable>::iterator viter;
        for(viter=tt.relvars.begin();viter!=tt.relvars.end();++viter)
        {
            tt.relvararray.push_back(*viter);
        }
    }
}

void CPN::TraArcTreeforVAR(meta *expnode, set<Variable> &relvars) {
    if(expnode == NULL)
        return;
    if(expnode->mytype == var)
    {
        map<string,VARID>::iterator viter;
        viter = mapVariable.find(expnode->myname);
        VARID vid = viter->second;
        relvars.insert(vartable[vid]);
        return;
    }
    if(expnode->leftnode!=NULL)
    {
        TraArcTreeforVAR(expnode->leftnode,relvars);
    }
    if(expnode->rightnode!=NULL)
    {
        TraArcTreeforVAR(expnode->rightnode,relvars);
    }
}

void CPN::printTransVar() {
    ofstream outvar("trans_var.txt",ios::out);
    for(int i=0;i<transitioncount;++i)
    {
        CPN_Transition &tt = transition[i];
        outvar<<transition[i].id<<":";
        set<Variable>::iterator iter;
        for(iter=tt.relvars.begin();iter!=tt.relvars.end();++iter)
        {
            outvar<<" "<<iter->id;
        }
        outvar<<endl;
    }
}

int CPN::getBENum() {
    int BENum=0;
    for(int i=0;i<transitioncount;++i)
    {
        int BtNum=1;
        set<Variable>::iterator iter;
        for(iter=transition[i].relvars.begin();iter!=transition[i].relvars.end();++iter)
        {
            int feconst = sorttable.usersort[iter->sid].feconstnum;
            BtNum *= feconst;
        }
        BENum += BtNum;
    }
    return BENum;
}

void Tokenscopy(Tokens &t1,const Tokens &t2,type tid,int PSnum)
{
    t1.initiate(t2.tokencount,tid,PSnum);
    if(tid == usersort)
    {
        COLORID cid;
        t2.tokens->getColor(cid);
        t1.tokens->setColor(cid);
    }
    else if(tid == productsort)
    {
        COLORID *cid = new COLORID[PSnum];
        t2.tokens->getColor(cid,PSnum);
        t1.tokens->setColor(cid,PSnum);
        delete [] cid;
    }
    else if(tid == finiteintrange)
    {
        cerr<<"[CPN.h\\Tokenscopy()].FiniteIntRange ERROR"<<endl;
        exit(-1);
    }
}