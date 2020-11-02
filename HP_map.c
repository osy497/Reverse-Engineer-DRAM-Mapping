#include <asm/unistd.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <linux/kernel-page-flags.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>


//#define PAGEMAP_ENTRY 8
#define degree 1024
//#define GET_BIT(X,Y) (X & ((uint64_t)1<<Y)) >> Y
//#define GET_PFN(X) X & 0x7FFFFFFFFFFFFF
//#define is_bigendian() ( (*(char*)&__endian_bit) == 0 )
#define sample_size 10000
#define iter_size 4096
#define total_size degree*degree*degree
#define list_size (total_size / iter_size)
#define V_bit 0b111111111111111111111111111111
//#define V_bit (~0)// & (~0b11111)
#define test_size 1000

#define ADDR (void *)(0x0UL)
#define LENGTH (1024UL * 1024 * 1024)
#define PROTECTION (PROT_READ | PROT_WRITE)
#define FLAGS (MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB)


int bit_size(uint64_t bit)
{
    int size = 0;
    int remainder;
    uint64_t bin = bit;
    while(bin != 0)
    {
        remainder = bin%2;
        bin = bin/2;
        size = size + remainder;
    }
    return size;
}
typedef struct ar{
    uint64_t bit[1024*16];
    int num;
} ar_;
typedef struct pot_bit{
    uint64_t bit[2048];
    uint64_t Rbit[2048];
    int num;
} pot_bit_;
ar_ ar;
pot_bit_ pot_bit;
uint64_t bank_bit[100];
uint64_t pot_bank_bit;
typedef struct orig_bit_{
    int bit[64];
    int num;
}orig_bit_;
orig_bit_ orig_bit;
void pot_bit_push(uint64_t bit){
    pot_bit.bit[pot_bit.num++] = bit;
    return ;
};
void init_orig_bit(){
    int i;
    for(i=0; i<64; i++)
        orig_bit.bit[i] = 0;
    orig_bit.num = 0;
    return ;
};
void push_orig_bit(int orig){
    orig_bit.bit[orig_bit.num++] = orig;
    return ;
};

void init_pot_bit(uint64_t bit){
    int i,j;
    int a=0, b=0, c=0;
    uint64_t bit_;
    for(i=0; i<2048; i++)
        pot_bit.Rbit[i] = 0;
    bit_ = bit;
    while(bit_){
        if(bit_&1){
            for(i=0; i<pot_bit.num; i++){
                c = (pot_bit.bit[i] & (1<<a)) >> a;
                pot_bit.Rbit[i] = pot_bit.Rbit[i] | (c<<b);

            }
        push_orig_bit(a);
        b++;
        }
        bit_ = bit_ >> 1;
        a++;
    }
    return ;

};
void init_ar(uint64_t bit){
    int i;
    for(i=0; i<1024*16; i++)
        ar.bit[i] = 0;
    for(i=0; i<(1<<bit_size(bit)); i++){
        ar.bit[i] = i;
    }
    ar.num = 1<<bit_size(bit);
    return ;
};
uint64_t answer[100];
int answer_num;

void init_answer(){
    int i;
    for(i=0; i<100; i++)
        answer[i] = 0;
    answer_num = 0;
    return ;
};

void push_answer(uint64_t bit){
    answer[answer_num++] = bit;
    return ;
};

int exist_in(uint64_t bit){
    int i;
    for(i=0; i<answer_num; i++){
        if(answer[i] == bit)
            return 1;
    }
    return 0;
};

uint64_t num_xor(int i, int size){
    uint64_t ret = 0;
    uint64_t j;
    int num = i;
    if(i==0)
        return 0;
    else{
        for(j=0; j<size; j++){
            ret = ret ^ (pot_bit.Rbit[j] * (num&1));
            num = num >> 1;
        }
    }
    return ret;
};

void extend_pot_bit(){
    int i;
    for(i=0; i<2048; i++)
        pot_bit.bit[i] = 0;
    int num = pot_bit.num;
    pot_bit.num = 0;
    uint64_t bit_;
    for(i=0; i<(1<<num); i++){
        bit_ = num_xor(i, num);
        if(!exist_in(bit_))
            pot_bit_push(bit_);

    }
    return ;
};

void get_answer(){
    int i, j;
    uint64_t bit, bit_;
    init_answer();
    int T;
    for(i=0; i<ar.num; i++){
        T=0;
        for(j=0; j<pot_bit.num; j++){
            bit_ = pot_bit.Rbit[j] & ar.bit[i];
            if((bit_size(bit_))%2 != 0){
                T = 1;
                break;
            }
        }
        if(T == 0)
            push_answer(ar.bit[i]);
    }
    return ;
};

uint64_t print_answer(uint64_t bit){
    int i = 0;
    printf("[ ");
    uint64_t bit_ = bit;
    uint64_t bbit = 0;
    while(bit_){
        if(bit_&1){
            printf("%d ", orig_bit.bit[i]);
            bbit = bbit | (1<<orig_bit.bit[i]);
        }
        bit_ = bit_ >> 1;
        i++;
    }

    printf("]");

    return bbit;
};

void cal(uint64_t *list){
    int i, size = 0;
    uint64_t pot_bit_ = 0;
    int j = 0;

    for(i=0; i<100; i++)
        if(list[i] != 0)
            pot_bit.bit[j++] = list[i];
    pot_bit.num = j;


    
    for(i=0; i<pot_bit.num; i++)
        pot_bit_ = (pot_bit_| pot_bit.bit[i]);

    
    init_pot_bit(pot_bit_);
    extend_pot_bit();
    init_ar(pot_bit_);
    get_answer();


    uint64_t bbit = 0;
    for(i=1; i<answer_num; i++){
        printf("!");
        bbit = print_answer(answer[i]);
        answer[i] = bbit;
        printf("\n");
    }
    
    return ;
};


typedef struct table_entry{
    uint64_t PA;
    uint64_t VA;
    bool clear;
} table_entry;

typedef struct pair{
    table_entry first;
    table_entry second;
    uint64_t diff_bit;
    int latency;
    int clear; // 1 = clear, 0 = not
} pair;

typedef struct add_bit{
    uint64_t bit[2500];
    int num;

}add_bit;

void push(uint64_t PA1, uint64_t VA1, uint64_t PA2, uint64_t VA2, int latency);
void init_pair();
uint64_t diff(uint64_t PA1, uint64_t PA2);
uint64_t time_stamp();
int check_latency(uint64_t VA1, uint64_t VA2);
int exist_in_pair(uint64_t diff_bit, int opt);
uint64_t convert_VA_into_PFN(uint64_t virt_addr);
void make_pair(int diff_num, int opt, uint64_t bit);
int pair_latency_avg();
void init_row_bit();
void init_col_bit();
void push_row_bit(uint64_t bit);
void push_col_bit(uint64_t bit);
void print_bit(int opt);

void init_PT();
char * make_PT(int opt);
void free_PT(char *ptr);

int one_diff_bit(uint64_t diff_bit);
void print_diff_bit(uint64_t diff_bit);
void print_test_pair();

void find_C_nodes(int diff_num);
void delete_(uint64_t PA, int size);

void find_B_nodes(int diff_num);
void make_pair_(uint64_t bit, int diff_num);

/***********************data****************************/
table_entry PT[list_size];
pair test_pair[sample_size];
uint64_t row_bit;
uint64_t col_bit;
int pair_size;
int PT_size;
uint64_t test_pair_bit;

uint64_t con_list[0x100];
int pagemap;
add_bit addbit[4];
int avg_latency;

uint64_t test_row_bit;
int HP_enable;
/*******************************************************/

void init_bank_bit();
void push_bank();
void concat(uint64_t bit, int num);
void push_con(uint64_t bit);
uint64_t *split(uint64_t bit);
int exist_(uint64_t bit);
void init_con();

int exist_in_bank(uint64_t bit);

void init_add_bit(int opt){
    addbit[opt].num = 0;
    int i;

    for(i=0; i<2500; i++){
        addbit[opt].bit[i] = 0;
    }

    return ;
}
void push_add_bit(int opt, uint64_t b){
    addbit[opt].bit[addbit[opt].num] = b;
    addbit[opt].num++;
    return ;
}
void make_add_bit(uint64_t bit){
    int i1, i2, i3, i4;
    for(i1=0; i1<4; i1++)
        init_add_bit(i1);


    int bit_size_;
    uint64_t push_bit;
    int j = 0;
    for(i1=0; i1<12; i1++)
        for(i2=i1; i2<12; i2++)
            for(i3=i2; i3<12; i3++)
                for(i4=i3; i4<12; i4++){

                    push_bit = (1<<i1)|(1<<i2)|(1<<i3)|(1<<i4);
                    bit_size_ = bit_size(push_bit&bit);

                    if((push_bit&bit) == push_bit)
                        push_add_bit(bit_size_ - 1, push_bit);
                    /* 
                    if(bit_size_== bit_size(push_bit))
                        if(bit_size_ <= size){
                            push_add_bit(bit_size_, push_bit);
                        }
                        */

                }
    return ;
    
}
int exist_in_bank(uint64_t bit)
{
    int i;
    for(i=0; i<100; i++){
        if(bank_bit[i] == bit)
            return 1;
        if(bit_size(bank_bit[i]&bit) > 1)
            if((bank_bit[i]&bit) == bank_bit[i])
                return ;
//            if((bank_bit[i]&bit == bank_bit[i]) || (bank_bit[i]&bit == bit))
//                return 1;
    }
    return 0;
}

void push_bank(uint64_t bit)
{
    int i;
    for(i=0; i<100; i++)
        if(bank_bit[i] == 0){
            bank_bit[i] = bit;
            pot_bank_bit = pot_bank_bit | bit;

            return ;
        }
    return ;
}

void init_bank_bit()
{
    int i;
    for(i=0; i<100; i++)
        bank_bit[i] = 0;
    pot_bank_bit = 0;
    return ;
}

void init_con()
{
    int i;
    for(i=0; i<0x100; i++)
        con_list[i] = 0;
    return ;
}

void find_B_nodes(int diff_num)
{
    char *ptr;
    int i, j, k;
    //    int test_size =1000;
    uint64_t diff_bit;
    uint64_t PA, PA_, VA, VA_;

    int high_num, low_num = 0;

    uint64_t rest_bit = V_bit & (~row_bit) & (~col_bit);
    uint64_t high_rest_bit = rest_bit & ~(0xFFF);
    uint64_t low_rest_bit = rest_bit & 0xFFF;


    printf("row bit : %"PRIx64", ", row_bit);
    printf("col bit : %"PRIx64", ", col_bit);
    printf("pot bank bit : %"PRIx64", ", pot_bank_bit);
    printf("rest bit : %"PRIx64"\n", rest_bit);



    init_PT();
    ptr = make_PT(1);

    init_pair();
    make_add_bit(low_rest_bit);

    for(i=0; i<test_size; i++){
        for(j=0; j<list_size; j++){

            diff_bit = diff(PT[i].PA, PT[j].PA);
            high_num = bit_size(diff_bit);

//            if(bit_size(diff_bit&row_bit) == 0){
            if((diff_bit&rest_bit) == diff_bit){
                if(high_num < diff_num){
                    for(k=0; k<addbit[diff_num-high_num-1].num; k++){
                        PA = PT[i].PA;
                        PA_ = PT[j].PA | addbit[diff_num-high_num-1].bit[k];
                        VA = PT[i].VA;
                        VA_ = PT[j].VA | addbit[diff_num-high_num-1].bit[k];
                        if(!exist_in_pair(PA^PA_, 0)){
                            push(PA, VA, PA_, VA_, check_latency(VA, VA_));

                        }
                    }
                }
                else if(diff_num == high_num){
                    PA = PT[i].PA;
                    PA_= PT[j].PA;
                    VA = PT[i].VA;
                    VA_= PT[j].VA;
                    if(!exist_in_pair(PA^PA_, 0)){
                        push(PA, VA, PA_, VA_, check_latency(VA, VA_));

                    }

                }
            }
        }

    }

//    printf("pair size : %d\n", pair_size);

//    avg_latency = pair_latency_avg();
    print_test_pair();


    j = 0;
    printf("bank latency average : %d\n", avg_latency);
    for(i=0; i<sample_size; i++)
        if(!(test_pair[i].clear))
            if(test_pair[i].latency > avg_latency)
                //                if(bit_size(test_pair[i].diff_bit)>1)
                if(!exist_in_bank(test_pair[i].diff_bit))
                    if((test_pair[i].diff_bit & V_bit) == test_pair[i].diff_bit)
                        push_bank(test_pair[i].diff_bit);

    munmap(ptr, total_size);
}

int exist_(uint64_t bit)
{
    int i;
    for(i=0; i<0x100; i++)
        if(con_list[i] == bit)
            return 1;
    return 0;
}

void concat(uint64_t bit, int num)
{
    int i, j, k = 0;
    int size = bit_size(bit);
    uint64_t bit_ = bit;
    uint64_t r[size];


    if(bit_size(bit)>num)
    {
        for(j=0; j<64; j++)
        {
            if(bit_ & 1 == 1){
                r[k++] = (1<<j);
            }
            bit_ = bit_/2;
            if(bit_ == 0)
                break;
        }

        for(i=0; i<size; i++)
            concat(bit&(~(r[i])), num);
    }
    else if(bit_size(bit) == num){
        if(!exist_(bit))
            push_con(bit);}
    else
    {
        return ;
    }
        return ;
}
void push_con(uint64_t bit){
    int i;
    for(i=0; i<100; i++)
    {
        if(con_list[i] == 0){
            con_list[i] = bit;
            return;
        }
    }
    return ;
}




void delete_(uint64_t PA, int size)
{
    int i;
    for(i=0; i<pair_size; i++)
    {
        if(test_pair[i].first.PA == PA)
            if(bit_size(test_pair[i].diff_bit) == size)
                test_pair[i].clear = 1;
    }
    return ;
}
void print_diff_bit(uint64_t diff_bit)
{
    int i, j = 0;
    int rand;
    uint64_t bit;
    bit = diff_bit;
    printf("diff bit : [ ");


    for(i=0; i<64; i++)
    {
        if(bit&1)
            printf("%d ",i);
        bit = bit/2;
        if(bit == 0)
            break;
    }
    printf("], ");
    return ;
}

int one_diff_bit(uint64_t diff_bit)
{
    int i;
    uint64_t bit=diff_bit;
    for(i=0; i<64; i++)
    {
        if(bit&1)
            return i;
        bit = bit /2;
    }
    return i;
}

void init_PT()
{
    int i;
    PT_size = 0;
    for(i=0;i<list_size;i++)
    {
        PT[i].clear = 1;
    }
    return ;
};
char *make_PT(int opt)
{
    uint64_t iter_size_, total_size_ = 0;
    uint64_t list_size_ = 0;
    int enable_HP = 1;
    char *ptr;

    if(HP_enable)
        ptr = mmap(ADDR, LENGTH, PROTECTION, FLAGS, 0, 0);
    else 
        ptr = mmap(ADDR, LENGTH, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0); 

    char *pointer = ptr;
    uint64_t i = 0;


    if(opt == 0){ // 32bit
        iter_size_ = 1;
        list_size_ = 4096; // page size
    }
    else{
        iter_size_ = 4096;
        list_size_ = (1024 * 1024 * 1024) / 4096;
    }



    for(i=0; i<list_size_; i++){
        memset(pointer, '1', 1);
        pointer = pointer + iter_size_;
    }


    uint64_t mask = 0xFFF;

    for(i=0; i<list_size_; i++)
    {
        if(PT[PT_size].clear)
        {
            PT[PT_size].VA = (unsigned long)ptr + i * iter_size_;
            if(HP_enable == 0) PT[PT_size].PA = (convert_VA_into_PFN(PT[PT_size].VA)<<12)|((PT[PT_size].VA) & mask);
            else if(HP_enable == 1) PT[PT_size].PA = PT[PT_size].VA;

            PT[PT_size].clear = 0;
            PT_size++;
        }
    }
    return ptr;
};
void free_PT(char *ptr)
{
    munmap(ptr, total_size);
    return;
};

void init_row_bit()
{
    row_bit = 0;
    test_row_bit = 0;
    return ;
};
void init_col_bit()
{
    col_bit = 0;
    return ;
};
void push_row_bit(uint64_t bit)
{
    row_bit = row_bit|bit;
    return ;
};
void push_col_bit(uint64_t bit)
{
    col_bit = col_bit|bit;
    return ;
};
void print_bit(int opt)
{
    int i;
    uint64_t bit = 0;
    if(opt == 0)
    {
        bit = row_bit;
        printf("row bit : ");
    }
    else if(opt == 1)
    {
        bit = col_bit;
        printf("column bit : ");
    }
    else if (opt == 2)
        printf("bank bit : \n");


    if(opt != 2){
        for(i=0; i<64; i++)
        {
            if(bit&1)
                printf("%d ",i);
            bit = bit / 2;
        }
        printf("\n");}
    else
    {
        for(i=0; i<100; i++)
            if(bank_bit[i] != 0){
                print_diff_bit(bank_bit[i]);
                printf("\n");
            }
    }
    return ;

};

void push(uint64_t PA1, uint64_t VA1, uint64_t PA2, uint64_t VA2, int latency)
{
    test_pair[pair_size].clear = 0;
    test_pair[pair_size].first.PA = PA1;
    test_pair[pair_size].first.VA = VA1;
    test_pair[pair_size].second.PA = PA2;
    test_pair[pair_size].second.VA = VA2;
    test_pair[pair_size].diff_bit = diff(PA1,PA2);
    test_pair[pair_size].latency = latency;
    test_pair_bit = test_pair_bit|test_pair[pair_size].diff_bit;
    pair_size++;
    return ;
};
void init_pair()
{
    int i;
    pair_size = 0;
    test_pair_bit = 0;
    for(i=0; i<sample_size; i++)
    {
        test_pair[i].clear = 1;
        test_pair[i].diff_bit = 0;
    }
    return ;
};

uint64_t diff(uint64_t PA1, uint64_t PA2)
{
    return PA1^PA2;
};
uint64_t time_stamp()
{
    uint64_t hi, lo;
    asm volatile(
            "rdtscp;"
            : "=a"(lo), "=d"(hi)
            );
    return (lo)|((hi)<<32);

};


int check_latency(uint64_t VA1, uint64_t VA2)
{
    int i;
    uint64_t start, end;
    uint64_t latency=0;
    char *a = (char *)VA1;
    char *b = (char *)VA2;
    int j=0;
    //    start = time_stamp();
    for(i = 0;i <1000; i++)
    {
        start = time_stamp();
        asm volatile
            (
             "mov (%0), %%r8;"
             "mov (%1), %%r9;"
             "clflush (%0);"
             "clflush (%1);"
             "mfence;"
             :: "q"(a), "p"(b)
            );
        end = time_stamp();

        if(end-start <= 1000)
        {
            latency = latency + (end-start);
            j++;
        }
    }

    //    end = time_stamp();
    //    latency = end-start;
    //    return latency/j;
    return latency / j;
};

int exist_in_pair(uint64_t diff_bit, int opt)
{
    int i;

    if(opt == 1)
    {
        if(test_pair_bit & diff_bit)
            return 1;
        else
            return 0;
    }
    else
    {
        for(i=0; i<pair_size; i++)
        {
            if(!(test_pair[i].clear))
                if(test_pair[i].diff_bit == diff_bit)
                    return 1;
        }
        return 0;
    }
};

uint64_t convert_VA_into_PFN( uint64_t virt_addr)
{
    uint64_t value;
    int got = pread(pagemap, &value, 8, ((uintptr_t)(virt_addr)/ 0x1000) * 8);
    assert(got == 8);
    uint64_t page_frame_number = value&((1ULL << 54)-1);
    return page_frame_number;
//    return virt_addr;
};

void make_pair(int diff_num, int opt, uint64_t bit)
{
    int i, j, k = 0;
    //    int test_size = 1000;
    uint64_t diff_bit = 0;
    uint64_t VA1, VA2, PA1, PA2 = 0;
    char *ptr;

//    printf("make pair \n");
//    printf("pair size : %d\n",pair_size);
    init_PT();
    ptr = make_PT(opt);

    for(i = 0; i< test_size; i++)
    {
        VA1 = PT[i].VA;
        PA1 = PT[i].PA;
        for(j= i+1; j< list_size; j++)
        {
            VA2 = PT[j].VA;
            PA2 = PT[j].PA;
            diff_bit = diff(PA1, PA2);
            if(!exist_in_pair(diff_bit,1))
                if(bit_size(diff_bit) == diff_num)
                    if(bit_size(diff_bit&bit) == diff_num){
                        test_row_bit = test_row_bit | diff_bit;
                        push(PA1,VA1, PA2,VA2, check_latency(VA1, VA2));
                    }
        }
    }
    munmap(ptr, total_size);
    return ;
};
int pair_latency_avg()
{
    int i, j = 0;
    int latency_avg = 0;
    for(i=0; i<pair_size; i++)
        if(!(test_pair[i].clear))
        {
            latency_avg = latency_avg + test_pair[i].latency;
            j++;
        }

//    printf("pair size : %d\n",j);
    if(pair_size == 0)
    {
        printf("error: no latency in test pair!\n");
        return 0;
    }
    return latency_avg/j;
}
void find_R_nodes()
{
    int i, latency_avg = 0;
    printf("init start\n");
    init_pair();
    printf("make test pair start\n");

    /************* 12 < bit******************/
//        while(!((test_pair_bit&V_bit) == V_bit)) {
    make_pair(1, 0, V_bit);
    make_pair(1, 1, V_bit);
//        }

    if(test_row_bit != V_bit)
        for(i=0; i<10; i++)
            printf("This results may have error\n");
    printf("start to analyze latency\n");

    /****************************************/

    latency_avg = pair_latency_avg();


    avg_latency = latency_avg + 50;

    printf("&&&&&&&&&&&&&&&&&&&& latency average : %d&&&&&&&&&&&&&&&&&&&&&\n", avg_latency);

    for(i=0; i<pair_size; i++)
        if(test_pair[i].latency > avg_latency)
            push_row_bit(test_pair[i].diff_bit);
    print_test_pair();
    printf("avg latency : %d\n",latency_avg);
    print_bit(0);

    return ;

};
void find_C_nodes(int diff_num)
{
    int i, j, k, latency_avg = 0;
    uint64_t not_R_bit = 0;
    char* ptr;
    not_R_bit = V_bit & (~row_bit);
    uint64_t PA1, PA2, VA1, VA2;
    uint64_t test_bit = 0;
    uint64_t not_C_bit = 0;
    int a = 0;

    printf("not r bit : %"PRIx64"\n",not_R_bit);

    /**************************************************/

    uint64_t pa1, pa2, va1, va2, diff_bit;
    init_PT();
    ptr = make_PT(1);
    init_pair();
    make_add_bit( not_R_bit);
    for(i=0; i<test_size; i++){
        for(j=0; j<list_size; j++){
            ;
        }
    }

    /**************************************************/

    munmap(ptr, total_size);

    init_PT();
    ptr = make_PT(1);
    init_pair();
    for(i=0; i<test_size; i++){
        for(j=0; j<list_size; j++){
            diff_bit = diff(PT[i].PA, PT[j].PA);
            if(bit_size(diff_bit) == 1)
                if(bit_size(diff_bit&row_bit) == 1)
                    if(!exist_in_pair(diff_bit, 0))
                        push(PT[i].PA, PT[i].VA, PT[j].PA, PT[j].VA, 0);
        }
    }

    a = pair_size;
    for(i=0; i<a; i++)
    {
        PA1 = test_pair[i].first.PA;
        PA2 = test_pair[i].second.PA;
        VA1 = test_pair[i].first.VA;
        VA2 = test_pair[i].second.VA;
        for(j=0; j<12; j++)
        {
            push(PA1, (VA1), (PA2|(1<<j)), ((VA2)|(1<<j)), check_latency(VA1, (VA2|(1<<j))));   
        }
        delete_(PA1, 1 /*size*/);
    }
    print_test_pair();



    munmap(ptr, total_size);


//    printf("pair size : %d\n", pair_size);
    printf("first : %"PRIx64", second : %"PRIx64"\n",row_bit, not_R_bit);
    init_PT();
    ptr = make_PT(1);
    for(i=0; i<test_size; i++)
        for(j=0; j<list_size; j++)
        {
            diff_bit = diff(PT[i].PA, PT[j].PA);
            if(bit_size(diff_bit) == 2)
                if(bit_size(diff_bit & not_R_bit) == 1)
                    if(!exist_in_pair(diff_bit, 0))
                    {
                        push(PT[i].PA, PT[i].VA, PT[j].PA, PT[j].VA, check_latency(PT[i].VA, PT[j].VA));
                        test_bit = test_bit | (diff_bit & ~(1<<one_diff_bit(row_bit)));
                    }
        }
    munmap(ptr, total_size);


    latency_avg = pair_latency_avg() + 10;

    for(i=0; i<sample_size; i++)
        if(!(test_pair[i].clear)){
            if(bit_size((test_pair[i].diff_bit) & row_bit) == 1){
                test_bit = (test_pair[i].diff_bit)&(~row_bit);
                if(test_pair[i].latency>avg_latency)
                    push_col_bit(test_bit);
                else
                    not_C_bit = not_C_bit|test_bit;
            }
        }

    col_bit = col_bit&~(not_C_bit);
    print_test_pair();
    printf("latency_avg : %d \n",latency_avg);
    print_bit(1);
}


void make_pair_(uint64_t bit, int diff_num)
{
    int i, j;
    //    int test_size =1000;
    uint64_t diff_bit;
    for(i=0; i<test_size; i++)
        for(j=0; j<list_size; j++)
        {
            diff_bit = diff(PT[i].PA, PT[j].PA);
            if(bit_size(diff_bit) == diff_num)
                if(bit_size(diff_bit & bit) == diff_num)
                    if(!exist_in_pair(diff_bit, 0))
                        push(PT[i].PA, PT[i].VA, PT[j].PA, PT[j].VA, check_latency(PT[i].VA, PT[j].VA));
        }
}


void print_test_pair()
{
    int i;

    for(i=0; i<sample_size; i++)
    {
        if(!(test_pair[i].clear)){
            printf("%d. ",i);
            printf("first : %"PRIx64", second : %"PRIx64", ", (test_pair[i].first.PA), (test_pair[i].second.PA));
            print_diff_bit(test_pair[i].diff_bit);
            printf("latency : %d", test_pair[i].latency);
            printf("\n");
        }
    }
//    printf("sample size : %d\n",pair_size);
};

void find_additional_nodes(int diff_num, uint64_t Rbit){

    uint64_t rest_bit = V_bit&(~row_bit)&(~col_bit)&(~pot_bank_bit);
    //    printf("row bit : %"RPIx64", col bit : %"PRIx64", pot bank bit : %"PRIx64"\n",row_bit, col_bit, pot_bank_bit);
    printf("row bit : %"PRIx64", ", row_bit);
    printf("col bit : %"PRIx64", ", col_bit);
    printf("pot bank bit : %"PRIx64", ", pot_bank_bit);
    printf("rest bit : %"PRIx64"\n", rest_bit);


    uint64_t low_rest_bit = rest_bit & 0xFFF;
    uint64_t high_rest_bit = rest_bit & ~(0xFFF);

    int size;
    uint64_t pa1, pa2, va1, va2, diff_bit;
    init_PT();
    char* ptr = make_PT(1);
    init_pair();
    int i, j, k, latency_avg = 0;
    int low_num, high_num = 0;
    make_add_bit(low_rest_bit);
    for(i=0; i<test_size; i++){
        for(j=0; j<list_size; j++){
            diff_bit = diff(PT[i].PA, PT[j].PA);
            high_num = bit_size(diff_bit&high_rest_bit);
            /*******************/

            if(diff_num > high_num){
                if(bit_size(diff_bit&Rbit)>0){
                    //                printf("diff bit : %"PRIx64", first : %"PRIx64", second : %"PRIx64"\n", diff_bit, diff_bit&high_rest_bit, diff_bit&~(row_bit));




                    if((diff_bit&high_rest_bit) == (diff_bit&~(Rbit))){
                        if(bit_size(diff_bit&~(Rbit)) < diff_num){
                            for(k=0; k<addbit[diff_num-high_num-1].num; k++){
                                pa1 = PT[i].PA;
                                pa2 = PT[j].PA | addbit[diff_num-high_num-1].bit[k];
                                va1 = PT[i].VA;
                                va2 = PT[j].VA | addbit[diff_num-high_num-1].bit[k];
                                if(!exist_in_pair(pa1^pa2, 0)){
                                    push(pa1, va1, pa2, va2, check_latency(va1, va2));
                                }


                            }

                        }
                        else if(bit_size(diff_bit&~(Rbit)) == diff_num){
                            pa1 = PT[i].PA;
                            pa2 = PT[j].PA;
                            va1 = PT[i].VA;
                            va2 = PT[j].VA;
                            if(!exist_in_pair(pa1^pa2, 0))
                                push(pa1, va1, pa2, va2, check_latency(va1, va2));

                        }
                    }

                }
                /********************/
            }
        }
    }
    print_test_pair();
    for(i=0; i<sample_size; i++){
        if(!(test_pair[i].clear))
            if(test_pair[i].latency > avg_latency){
                diff_bit = test_pair[i].diff_bit&~(Rbit);
                if(!exist_in_bank(diff_bit))
                    push_bank(diff_bit);
            }
    }

    munmap(ptr, total_size);



    return ;
};
uint64_t select__(uint64_t bit, uint64_t *list, int num_){
    int j;
    uint64_t i = 0;
    int min_size = 0;
    uint64_t min_num = 0;
    int size = 0;
    uint64_t num = 0;
    uint64_t bit_ = 0;
    int sum, max_sum = 0;
    int count = 0;
    for(i=1; i< (1<<bit_size(bit)); i++){
        if(bit_size(i) == num_){
            size = 0;
            bit_ = 0;
            sum = 0;
            count = 0;
            for(j=0; j<100; j++){
                if(list[j]){
                    if(((1<<j)&i) == (1<<j)){
                        size = size + bit_size(list[j]);
                        bit_ = bit_ | list[j];
                        sum = sum + list[j];
                        count++;
                    }
                }
            }
            if((bit_ == bit) && (count == num_)){
                if(min_size == 0){
                    min_size = size;
                    min_num = i;
                    max_sum = sum;
                }

                else if(min_size > size){
                    min_size = size;
                    min_num = i;
                    max_sum = sum;
                }
                else if((min_size == size) && (sum > max_sum)){
                    min_size = size;
                    min_num = i;
                    max_sum = sum;
                }
            }



        }
    }
    return min_num;
};

int same_bank(uint64_t *list, uint64_t bit){
    int i;
    uint64_t bank1, bank2 = 0;
    for(i=0; i<100; i++){
        if(list[i]){
            if((bit_size(list[i] & bit) % 2) != 0)
                return 0;
        }
    }
    return 1;

};
int exist__(uint64_t *list, uint64_t bit){
    int i, ret = 0;
    for(i=0; i<100; i++)
        if(list[i])
            if(list[i] == bit)
                return 1;
    return 0;

};
uint64_t get_correct_bit(uint64_t bit, uint64_t pot_bit){
    uint64_t bit_ = bit;
    uint64_t pot_bit_ = pot_bit;
    uint64_t ret = 0;
    int pot_num, bit_num = 0;
    int i, j = 0;
    for(i=0; i<64; i++){
        while(pot_bit_){
            if(pot_bit_&1){
                ret = ret | ((bit_&1)<<j);
                pot_bit_ = pot_bit_ >> 1;
                j++;
                break;
            }
            pot_bit_ = pot_bit_ >> 1;
            j++;
        }

        bit_ = bit_ >> 1;
    }
    return ret;
};
int exist_bit(uint64_t *diff_bit, uint64_t bit){
    int i, ret = 0;
    for(i=0; i<100; i++)
        if((diff_bit[i]&bit) != 0)
            ret++;
    return ret;

};

uint64_t row_from_bank(uint64_t *diff_bit, uint64_t *bank_bit){
    uint64_t pot_bit = 0;
    uint64_t bit;
    int i;
    int candi_num = 0;
    int diff_bit_num = 0;//sizeof(diff_bit) / sizeof(uint64_t);
    int bank_bit_num = 0;//sizeof(bank_bit) / sizeof(uint64_t);

    for(i=0; i<100; i++)
        if(diff_bit[i])
            diff_bit_num++;
    for(i=0; i<100; i++)
        if(bank_bit[i])
            bank_bit_num++;


    for(i=0; i<diff_bit_num; i++)
        pot_bit = pot_bit | diff_bit[i];

    uint64_t candi[1000] = {0}; // candidate row bit
    //pick n row bit and put into candi

    for(i=0; i<(1<<bit_size(pot_bit)); i++){
        bit = get_correct_bit(i, pot_bit);
        if(exist_bit(diff_bit, bit) == diff_bit_num)
            candi[candi_num++] = bit;
    }

    //    printf("exist : %d\n", exist__(diff_bit, (1<<8)|(1<<15)|(1<<19)));
    for(i=0; i<1000; i++){
        if(same_bank(bank_bit, candi[i]) && !exist__(diff_bit, candi[i]))
            candi[i] = 0;
    }

    int j = 0;
    int min_size = 0;
    uint64_t min_bit = 0;
    for(i=0; i<1000; i++){
        if(candi[i]){
            if(min_size == 0){
                min_bit = candi[i];
                min_size = bit_size(candi[i]);
            }
            else if(min_size > bit_size(candi[i])){
                min_bit = candi[i];
                min_size = bit_size(candi[i]);
            }
            else if((min_size == bit_size(candi[i])) && (min_bit < candi[i]))
                min_bit = candi[i];
        }
    }


    return min_bit;
};


int main(int argc, char* argv[])
{
    if(argc != 2){
        fputs("error! please input the only number of bank bits\n", stderr);
        exit(1);
    }

    int i;
    init_row_bit();
    init_col_bit();
    init_bank_bit();
    int pid = getpid();
    char* ptr = mmap(ADDR, LENGTH, PROTECTION, FLAGS, 0, 0);

    if(ptr == MAP_FAILED){
        HP_enable = 0;
        printf("HP un-available\n");
    }
    else{
        HP_enable = 1;
        munmap(ptr, LENGTH);
        printf("HP available\n");
        
    }




    
    pagemap = open("/proc/self/pagemap", O_RDONLY);

    printf("start R nodes\n");
    find_R_nodes();
    find_R_nodes();
    printf("start C nodes\n");
    find_C_nodes(3);

    printf("B nodes\n");

    find_B_nodes(2);
    printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    find_B_nodes(3);
    find_B_nodes(4);

    printf("previous bank bit\n");
    print_bit(2);

    int j = 0;
    int k, n, m;
    int count = 0;
    uint64_t bit;
    uint64_t Rbit, Rbit_;
    uint64_t fake_bit[100] = {0};
    int fake_num = 0;
    uint64_t fakebit[100] = {0};
    int fakenum = 0;


    /*********************
      col_bit = 0x20000c7f;
      row_bit = 0x1f800000;
      avg_latency = 500;


      bank_bit[0] = 0x110000;
      bank_bit[1] = 0x220000;
      bank_bit[2] = 0x8a000;
      bank_bit[3] = 0x88100;
      bank_bit[4] = 0x88200;
      bank_bit[5] = 0x89000;
      bank_bit[6] = 0x440100;
      bank_bit[7] = 0x440200;
      bank_bit[8] = 0x441000;
      bank_bit[9] = 0x442000;

      for(i=0; i<10; i++)
      pot_bank_bit = pot_bank_bit | bank_bit[i];


     *********************/

    uint64_t Rrow = row_bit;



    for(i=0; i<100; i++){
        if(bank_bit[i]){
            for(j=0; j<100; j++)
                if(j != i)
                    if((bank_bit[i]&bank_bit[j]) != 0){
                        fake_bit[fake_num++] = bank_bit[i];
                        break;
                    }

            if(j == 100){
                fakebit[fakenum++] = bank_bit[i];
                bank_bit[i] = 0;
            }
        }
    }


    for(i=0; i<100; i++)
        for(j=0; j<100; j++)
            if(bank_bit[i] == fake_bit[j])
                bank_bit[i] = 0;


    for(i=0; i<100; i++){
        if(bit_size(fakebit[i]) == 2){
            push_bank(fakebit[i]);
            k = 0;
            count = 0;
            bit = fakebit[i];
            while(bit){
                if(bit&1){
                    if(count == 1)
                        push_row_bit(1<<k);
                    count++;
                }
                k++;
                bit = bit >> 1;
            }
        }
        else if(bit_size(fakebit[i]) == 3){
            bit = fakebit[i];
            k = 0;
            count = 0;
            Rbit = 0;
            Rbit_ = 0;
            printf("bit : %"PRIx64"\n", bit);
            while(bit){
                if(bit&1){
                    if(count == 0)
                        Rbit = Rbit | (1<<k);
                    else if(count == 1)
                        Rbit_ = Rbit_ | (1<<k);
                    else if(count == 2){
                        Rbit = Rbit | (1<<k);
                        Rbit_ = Rbit_ | (1<<k);
                        push_row_bit(1<<k);
                        push_bank(Rbit);
                        push_bank(Rbit_);
                        break;
                    }
                    count++;
                }
                bit = bit >> 1;
                k++;
            }
        }
    }

    printf("start cal \n");
    cal(fake_bit);
    printf("start to find additional node\n");

    
    
    find_additional_nodes(2, Rrow);
    uint64_t rest_bit = V_bit & (~row_bit) & (~col_bit) & (~pot_bank_bit);
    count = 0;
    while(rest_bit){
        if(rest_bit&1)
            push_bank(1<<count);
        rest_bit = rest_bit >> 1;
        count++;

    }

    int num = 0;
    for(i=0; i<100; i++)
        if(bank_bit[i])
            num++;
    uint64_t pot = 0;
    for(i=0; i<100; i++)
        if(answer[i])
            pot = pot | answer[i];


    int rest_num = 0;
    for(i=0; i<100; i++)
        if(bank_bit[i])
            rest_num++;
    int bank_bit_num = atoi(argv[1]);
    rest_num = bank_bit_num - rest_num;

    uint64_t s = select__(pot, answer, rest_num);
    i = 0;
    j = 0;


    uint64_t BanBit[100] = {0};
    uint64_t bbb = 0;
    uint64_t bbit;
    while(s){
        if(s&1){
            printf("answer : %"PRIx64"\n", answer[i]);
            push_bank(answer[i]); //orig_bit.bit
            BanBit[bbb++] = answer[i];
        }
        s = s >> 1;
        i++;
    }

    push_row_bit(row_from_bank(fake_bit, BanBit));



    printf("**************************************\n");
    printf("**************************************\n");

    print_bit(0);
    print_bit(1);
    print_bit(2);
    if(!HP_enable) printf(" ------> This result may contain faults. Please re-execute this code to re-check.\n");
    printf("**************************************\n");
    printf("**************************************\n");


    return 0;
}

