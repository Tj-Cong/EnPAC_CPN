#include <sys/time.h>
#include "CPN_Product.h"
#include "BA/SBA.h"
#include "BA/xml2ltl.h"
using namespace std;
#define VMRSS_LINE 17
#define VMSIZE_LINE 13
#define PROCESS_ITEM 14

NUM_t placecount;
NUM_t transitioncount;
NUM_t varcount;
CPN *cpnet;
CPN_RG *cpnRG;
extern bool ready2exit;

size_t  heap_malloc_total, heap_free_total,mmap_total, mmap_count;
void print_info() {
    struct mallinfo mi = mallinfo();
    printf("count by itself:\n");
    printf("\033[31m\theap_malloc_total=%lu heap_free_total=%lu heap_in_use=%lu\n\tmmap_total=%lu mmap_count=%lu\n",
           heap_malloc_total*1024, heap_free_total*1024, heap_malloc_total*1024-heap_free_total*1024,
           mmap_total*1024, mmap_count);
    printf("count by mallinfo:\n");
    printf("\theap_malloc_total=%lu heap_free_total=%lu heap_in_use=%lu\n\tmmap_total=%lu mmap_count=%lu\n\033[0m",
           mi.arena, mi.fordblks, mi.uordblks,
           mi.hblkhd, mi.hblks);
//    malloc_stats();
}

unsigned int get_proc_mem(unsigned int pid){

    char file_name[64]={0};
    FILE *fd;
    char line_buff[512]={0};
    sprintf(file_name,"/proc/%d/status",pid);

    fd =fopen(file_name,"r");
    if(nullptr == fd){
        return 0;
    }

    char name[64];
    int vmrss;
    for (int i=0; i<VMRSS_LINE-1;i++){
        fgets(line_buff,sizeof(line_buff),fd);
    }

    fgets(line_buff,sizeof(line_buff),fd);
    sscanf(line_buff,"%s %d",name,&vmrss);
    fclose(fd);

    return vmrss;
}

double get_time() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec + t.tv_usec / 1000000.0;
}

int main() {
    cout << "=================================================" << endl;
    cout << "=====This is our tool-enPAC for the MCC'2020=====" << endl;
    cout << "=================================================" << endl;

    char Ffile[50] = "LTLFireability.xml";
    char Cfile[50] = "LTLCardinality.xml";
    convertf(Ffile);
    convertc(Cfile);

    int formula_num = 32;
    double starttime, endtime;
    pid_t mypid = getpid();
    /**************************NET*****************************/
    CPN *cpn = new CPN;
    char filename[] = "model.pnml";
    cpn->getSize(filename);
//    cpn->printSort();
    cpn->readPNML(filename);
    cpn->getRelVars();
//    cpn->printTransVar();
//    cpn->printVar();
//    cpn->printCPN();
    cpn->setGlobalVar();
    cpnet = cpn;
    cout<<"Place:"<<cpn->placecount<<endl;
    cout<<"Transition:"<<cpn->transitioncount<<endl;
    cout<<"BENum:"<<cpn->getBENum()<<endl;

    CPN_RG *crg;
//    crg = new CPN_RG;
//    CPN_RGNode *initnode = crg->CPNRGinitialnode();
//    crg->Generate(initnode);
//    cout<<"Hash Conflict Times:"<<crg->hash_conflict_times<<endl;
//    delete crg;
//    delete cpn;
    string S, propertyid; //propertyid stores names of LTL formulae
    char *form = new char[200000];     //store LTL formulae
    ofstream outresult("boolresult.txt", ios::out);  //outresult export results to boolresult.txt

    ifstream read("LTLCardinality.txt", ios::in);
    if (!read) {
        cout << "error!";
        getchar();
        exit(-1);
    }

    //print_info();

    while (getline(read, propertyid, ':')) {
        getline(read, S);
        int len = S.length();
        if (len >= 200000) {
            outresult << '?';
            cout << "FORMULA " + propertyid + " " + "CANNOT_COMPUTE" << endl;
            continue;
        }
        strcpy(form, S.c_str());
        //lexer
        Lexer *lex = new Lexer(form, S.length());
        //syntax analysis
        Syntax_Tree *ST;
        ST = new Syntax_Tree;
        formula_stack Ustack;
        ST->reverse_polish(*lex);
        ST->build_tree();

        ST->simplify_LTL(ST->root->left);

        ST->negconvert(ST->root->left, Ustack);
        ST->computeCurAP(ST->root->left);
        delete lex;

        crg = new CPN_RG;
        cpnRG = crg;

        TGBA *Tgba;
        Tgba = new TGBA;
        Tgba->CreatTGBA(Ustack, ST->root->left);
        delete ST;

        TBA *tba;
        tba = new TBA;
        tba->CreatTBA(*Tgba, Ustack);
        delete Tgba;
        string filename = propertyid + ".txt";

        SBA *sba;
        sba = new SBA;
        sba->CreatSBA(*tba);
        sba->Simplify();
        sba->Compress();
        sba->ChangeOrder();
        sba->PrintSBA();
        delete tba;

        starttime = get_time();
        CPN_Product_Automata *product = new CPN_Product_Automata(sba);
        cout<<get_proc_mem(getpid())<<endl;
        ready2exit = false;
        product->ModelChecker(propertyid);
        endtime = get_time();
        int ret = product->getresult();
        outresult << (ret == -1 ? '?' : (ret == 0 ? 'F' : 'T'));
        cout<<"RUNTIME:"<<endtime-starttime<<" NODECOUNT:"<<crg->nodecount<<" MEM:"<<get_proc_mem(mypid)<<endl;
        delete product;
        delete sba;
        delete crg;
        formula_num--;
        //print_info();
    }

    outresult<<endl;

    ifstream readF("LTLFireability.txt", ios::in);
    if (!readF) {
        cout << "error!";
        getchar();
        exit(-1);
    }

    //print_info();

    while (getline(readF, propertyid, ':')) {
        getline(readF, S);
        int len = S.length();
        if (len >= 20000) {
            outresult << '?';
            cout << "FORMULA " + propertyid + " " + "CANNOT_COMPUTE" << endl;
            continue;
        }
        strcpy(form, S.c_str());
        //lexer
        Lexer *lex = new Lexer(form, S.length());
        //syntax analysis
        Syntax_Tree *ST;
        ST = new Syntax_Tree;
        formula_stack Ustack;
        ST->reverse_polish(*lex);
        ST->build_tree();

        ST->simplify_LTL(ST->root->left);

        ST->negconvert(ST->root->left, Ustack);
        ST->computeCurAP(ST->root->left);
        delete lex;

        crg = new CPN_RG;
        cpnRG = crg;

        TGBA *Tgba;
        Tgba = new TGBA;
        Tgba->CreatTGBA(Ustack, ST->root->left);
        delete ST;

        TBA *tba;
        tba = new TBA;
        tba->CreatTBA(*Tgba, Ustack);
        delete Tgba;
        string filename = propertyid + ".txt";

        SBA *sba;
        sba = new SBA;
        sba->CreatSBA(*tba);
        sba->Simplify();
        sba->Compress();
        sba->ChangeOrder();
        delete tba;

        starttime = get_time();
        CPN_Product_Automata *product = new CPN_Product_Automata(sba);
        ready2exit = false;
        product->ModelChecker(propertyid);
        endtime = get_time();
        int ret = product->getresult();
        outresult << (ret == -1 ? '?' : (ret == 0 ? 'F' : 'T'));
        cout<<"RUNTIME:"<<endtime-starttime<<" NODECOUNT:"<<crg->nodecount<<" MEM:"<<get_proc_mem(mypid)<<endl;

        delete product;
        delete sba;
        delete crg;
        formula_num--;
    }
    delete cpn;
    return 0;
}