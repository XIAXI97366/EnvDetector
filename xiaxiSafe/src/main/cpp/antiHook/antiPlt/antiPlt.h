//
// Created by 97366 on 2024/6/19.
//

#ifndef CHECKROM_ANTIPLT_H
#define CHECKROM_ANTIPLT_H

#include "../../util/helper/helper.h"

#if defined(__LP64__)
#define ELF_R_SYM(info)  ELF64_R_SYM(info)
#define ELF_R_TYPE(info) ELF64_R_TYPE(info)
#define MULTIPLE_INHERITANCE_SIZE 16                 //gcc和clang在面对多重继承的时候额外在类中申请的空间
typedef uint64_t UPOINT;
#else
#define ELF_R_SYM(info)  ELF32_R_SYM(info)
#define ELF_R_TYPE(info) ELF32_R_TYPE(info)
typedef uint32_t UPOINT;
#define MULTIPLE_INHERITANCE_SIZE 8                  //gcc和clang在面对多重继承的时候额外在类中申请的空间
#endif


#if defined(__arm__)
#define ELF_R_GENERIC_JUMP_SLOT R_ARM_JUMP_SLOT      //.rel.plt
#define ELF_R_GENERIC_GLOB_DAT  R_ARM_GLOB_DAT       //.rel.dyn
#define ELF_R_GENERIC_ABS       R_ARM_ABS32          //.rel.dyn
#elif defined(__aarch64__)
#define ELF_R_GENERIC_JUMP_SLOT R_AARCH64_JUMP_SLOT
#define ELF_R_GENERIC_GLOB_DAT  R_AARCH64_GLOB_DAT
#define ELF_R_GENERIC_ABS       R_AARCH64_ABS64
#elif defined(__i386__)
#define ELF_R_GENERIC_JUMP_SLOT R_386_JMP_SLOT
#define ELF_R_GENERIC_GLOB_DAT  R_386_GLOB_DAT
#define ELF_R_GENERIC_ABS       R_386_32
#elif defined(__x86_64__)
#define ELF_R_GENERIC_JUMP_SLOT R_X86_64_JUMP_SLOT
#define ELF_R_GENERIC_GLOB_DAT  R_X86_64_GLOB_DAT
#define ELF_R_GENERIC_ABS       R_X86_64_64
#endif

typedef struct soInfo{
    char soName[BUFFER_LEN];
    UPOINT soBegin;           //so的符合属性的段的范围起始
    UPOINT soEnd;             //so的符合属性的段的范围结束
    struct soInfo *next;
}SOINFO;

typedef struct edi{
    //elf_hdr
    ElfW(Ehdr) *elf_hdr;

    //segment
    ElfW(Off) elf_phoff;
    ElfW(Half) elf_phentsize;
    ElfW(Half) elf_phnum ;
    ElfW(Phdr) *elf_phdr;
    ElfW(Phdr) **elf_loadSegAry;
    size_t loadCount;

    ElfW(Phdr) *elf_dynamic;

    ElfW(Addr)  bias_addr;

    const char *strtab; //.dynstr (string-table)
    size_t strsz;   //.dynstr size
    ElfW(Sym)  *symtab; //.dynsym (symbol-index to string-table's offset)

    ElfW(Addr)  relplt; //.rel.plt or .rela.plt
    ElfW(Word)  relplt_sz;

    ElfW(Addr)  reldyn; //.rel.dyn or .rela.dyn
    ElfW(Word)  reldyn_sz;

    ElfW(Addr)  relandroid; //android compressed rel or rela
    ElfW(Word)  relandroid_sz;

    //for ELF hash
    uint32_t   *bucket;
    uint32_t    bucket_cnt;
    uint32_t   *chain;
    uint32_t    chain_cnt; //invalid for GNU hash

    //append for GNU hash
    uint32_t    symoffset;
    ElfW(Addr) *bloom;
    uint32_t    bloom_sz;
    uint32_t    bloom_shift;

    int         is_use_rela;
    int         is_use_gnu_hash;
    int         is_parse_mem;

    size_t memSoSize;                   //maps中so的大小
    SOINFO *needLibHdr;                 //自身so需要的第三方的so信息
    char realSoPath[BUFFER_LEN];        //maps中该so的真实路径
} elf_dyn_info;

//1、计算offset + base 是否超过了模块范围
//2、对比源文件所记录的偏移

class antiPlt {
public:
    antiPlt();
    ~antiPlt();
    elf_dyn_info *elf_parse_mem_so(const char *soName);
    int check_rel_plt(elf_dyn_info *self);
    int check_rel_dyn(elf_dyn_info *self);
    int check_rel_android(elf_dyn_info *self);
private:
    UPOINT rva2fa(elf_dyn_info *self, UPOINT rva);
    int mapping_file(elf_dyn_info *self);
    void *get_maps_so_base(const char *soName, elf_dyn_info *self);
    size_t get_so_size(elf_dyn_info *self);
    elf_dyn_info *elf_parse_mmap_so(elf_dyn_info *self);
    ElfW(Word) get_dynamic_data_size(UPOINT base, UPOINT offset);
    int get_so_needed_libs(elf_dyn_info *self);          //获取该so中所需要的lib
    int parse_segment(elf_dyn_info *self, UPOINT base);
    int parse_dynamic_segment(elf_dyn_info *self, ElfW(Dyn *) data, ElfW(Word) dataSize);
    int parse_mmap_dynamic_segment(elf_dyn_info *self, UPOINT base);     //解析通过mmap将映射到内存中的so的 dynamic_segment
    int parse_mem_dynamic_segment(elf_dyn_info *self, UPOINT base);      //解析maps中的so的 dynamic_segment

    //检查so的.rel.plt(.rela.plt) 中的导入函数地址是否被第三方so Hook
    int check_mem_rel_plt_import_func_is_hooked(elf_dyn_info *self, UPOINT importAddr);
    //检查so的.rel.dyn(.rela.dyn) 中的全局变量地址是否被第三方so Hook
    int check_mem_rel_dyn_value_is_hooked(elf_dyn_info *self, UPOINT valueAddr);
    //检查出自于so自身的函数、全局变量 在.rel.plt(.rela.plt).rel.dyn(.rela.dyn)表项中是否处于Hook状态
    int check_rel_customize_sym_is_hooked(elf_dyn_info *mmap, elf_dyn_info *mem, int plt_or_dyn, UPOINT symAddr, size_t num);

    //.hash 用于解析.hash节表项
    int elf_hash_lookup(elf_dyn_info *self, const char *symName, uint32_t *symIdx);
    //.gnu.hash 用于解析.gnu.hash节表项
    int elf_gnu_hash_lookup_def(elf_dyn_info *self, const char *symbol, uint32_t *symIdx);
    int elf_gnu_hash_lookup_undef(elf_dyn_info *self, const char *symbol, uint32_t *symIdx);
    int elf_gnu_hash_lookup(elf_dyn_info *self, const char *symName, uint32_t *symidx);
    //自动判断使用hash还是gnuhash算法
    int elf_find_symidx_by_name(elf_dyn_info *self, const char *symbol, uint32_t *symIdx);
public:
    elf_dyn_info *memSelf = nullptr;
    elf_dyn_info *mmapSelf = nullptr;
private:
    char memSoName[BUFFER_LEN] = {0};
    size_t mmapSoSize = 0;
};

#endif //CHECKROM_ANTIPLT_H


