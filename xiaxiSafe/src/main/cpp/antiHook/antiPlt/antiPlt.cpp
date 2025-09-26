//
// Created by 97366 on 2024/6/19.
//
#include "antiPlt.h"

antiPlt::antiPlt() {
    if (nullptr == memSelf){
        memSelf = new elf_dyn_info();
        memset(memSelf, 0, sizeof(elf_dyn_info));
        memSelf->is_parse_mem = 1;
    }
    if (nullptr == mmapSelf){
        mmapSelf = new elf_dyn_info();
        memset(mmapSelf, 0, sizeof(elf_dyn_info));
    }
}

antiPlt::~antiPlt(){
    if (nullptr != mmapSelf){
        if (nullptr != mmapSelf->elf_hdr){
            if (nullptr != mmapSelf->elf_loadSegAry){
                delete [] mmapSelf->elf_loadSegAry;
                mmapSelf->elf_loadSegAry = nullptr;
            }
            munmap(mmapSelf->elf_hdr, mmapSoSize);
            mmapSelf->elf_hdr = nullptr;
        }
    }

    if (nullptr != memSelf){
        if (nullptr != memSelf->elf_loadSegAry){
            delete [] memSelf->elf_loadSegAry;
            memSelf->elf_loadSegAry = nullptr;
        }
        //释放链表
        if (nullptr != memSelf->needLibHdr){
            SOINFO *hdr = memSelf->needLibHdr;
            SOINFO *tmp = nullptr;
            while(hdr){
                if (nullptr == hdr->next){
                    delete hdr;
                    memSelf->needLibHdr = nullptr;
                    break;
                }
                tmp = hdr->next;
                delete hdr;
                hdr = tmp;
            }
        }
        delete mmapSelf;
        mmapSelf = nullptr;
    }
}

elf_dyn_info *antiPlt::elf_parse_mem_so(const char *soName) {
    strncpy(memSoName, soName, strlen(soName));
    memSelf->elf_hdr = (ElfW(Ehdr) *)get_maps_so_base(soName, memSelf);
    if (nullptr != memSelf->elf_hdr){
        if (!parse_segment(memSelf, (UPOINT)memSelf->elf_hdr)){
            LOGD("解析maps so的段错误");
            return nullptr;
        }else{
            return memSelf;
        }
    }else{
        LOGD("maps中未找到该so或是被隐藏");
        return nullptr;
    }
}

elf_dyn_info *antiPlt::elf_parse_mmap_so(elf_dyn_info *self) {
    if (!parse_segment(self, (UPOINT)self->elf_hdr)){
        LOGD("解析mmap so的段错误");
        return nullptr;
    }else{
        return mmapSelf;
    }
}

void *antiPlt::get_maps_so_base(const char *soName, elf_dyn_info *self) {
    FILE *fp = nullptr;
    UPOINT end = 0;
    UPOINT base = 0;
    char buffer[1024] = {0};

    fp = fopen("/proc/self/maps", "r");
    if (fp){
        while (fgets(buffer, sizeof(buffer), fp) ){
            if (sub_strstr(buffer, soName) && (sub_strstr(buffer, "r-xp") ||
                sub_strstr(buffer, "--xp") || sub_strstr(buffer, "r--p")) ){
                sscanf(buffer, "%lx-%lx %*4s %*lx %*s %*s %255[^] ", &base, &end, self->realSoPath);
                if (*(unsigned int*)base == 0x464C457F){
                    break;
                }
            }
            memset(buffer, 0, sizeof(buffer));
        }
        fclose(fp);
        return (void*)base;
    }
    return nullptr;
}

int antiPlt::parse_segment(elf_dyn_info *self, UPOINT base) {
    int loadNum = 0;
    self->elf_phoff = self->elf_hdr->e_phoff;
    self->elf_phnum = self->elf_hdr->e_phnum;
    self->elf_phentsize = self->elf_hdr->e_phentsize;
    self->elf_phdr = (ElfW(Phdr) *)((UPOINT)base + self->elf_phoff);
    void *tmp = self->elf_phdr;

    for (int i = 0; i < self->elf_phnum; ++i) {
        if(PT_LOAD == ((ElfW(Phdr) *)tmp)->p_type){
            loadNum++;
        }else if(PT_DYNAMIC == ((ElfW(Phdr) *)tmp)->p_type){
            self->elf_dynamic = ((ElfW(Phdr) *)tmp);
        }
        tmp = (ElfW(Phdr) *)((UPOINT)tmp + self->elf_phentsize);
    }

    //elf_loadSegAry数组指向复数个load段
    if (0 != loadNum){
        self->elf_loadSegAry = new ElfW(Phdr *)[loadNum];
        if (nullptr != self->elf_loadSegAry){
            void *tmp = self->elf_phdr;
            loadNum = 0;
            for (int i = 0; i < self->elf_phnum; ++i) {
                if (PT_LOAD == ((ElfW(Phdr) *)tmp)->p_type){
                    self->elf_loadSegAry[loadNum] = ((ElfW(Phdr) *)tmp);
                    loadNum++;
                }
                tmp = (ElfW(Phdr) *)((UPOINT)tmp + self->elf_phentsize);
            }
        }
    }
    self->loadCount = loadNum;

    if (nullptr != self->elf_dynamic && 1 == self->is_parse_mem){
        //解析内存中的DYNAMIC的段
        if (!parse_mem_dynamic_segment(self, base)){
            LOGD("maps so android rel/rela 格式错误，停止解析\n");
            return 0;
        }
        return 1;
    }else if(nullptr != self->elf_dynamic && 0 == self->is_parse_mem){
        //解析映射的DYNAMIC的段
        if (!parse_mmap_dynamic_segment(self, base)){
            LOGD("mmap so android rel/rela 格式错误，停止解析\n");
            return 0;
        }
        return 1;
    }else{
        LOGD("获取Dynamic的地址失败");
        return 0;
    }
}

ElfW(Word) antiPlt::get_dynamic_data_size(UPOINT base, UPOINT offset) {
    ElfW(Word) size = 0;
    ElfW(Dyn *) data = nullptr;

    data = (ElfW(Dyn *))(offset + base);
    while(true){
        if (0 == data->d_tag){
            size += sizeof(ElfW(Dyn));
            return size;
        }
        size += sizeof(ElfW(Dyn));
        data = (ElfW(Dyn *))((UPOINT)data + sizeof(ElfW(Dyn)));
    }
}

int antiPlt::parse_dynamic_segment(elf_dyn_info* self, ElfW(Dyn *) data, ElfW(Word) dataSize) {
    uint32_t *raw = nullptr;
    for(int i = 0; i < (dataSize / sizeof(ElfW(Dyn))); i++){
        switch(data->d_tag) {
            case DT_NULL:{
                break;
            }
            case DT_STRTAB:{
                //指向.dynstr节表项的节区数据(RVA)的地址
                self->strtab = (const char *)(self->bias_addr + data->d_un.d_ptr);
                break;
            }
            case DT_STRSZ:{
                //.dynstr节表项的节区数据的大小
                self->strsz = (data->d_un.d_val);
                break;
            }
            case DT_SYMTAB:{
                //指向.dynsym节表项的节区数据(RVA)的地址
                self->symtab = (ElfW(Sym) *)(self->bias_addr + data->d_un.d_ptr);
                break;
            }
            case DT_PLTREL:
                //use rel or rela?
                self->is_use_rela = (data->d_un.d_val == DT_RELA ? 1 : 0);
                break;
            case DT_JMPREL:{
                //指向.rel.plt节表项的节区数据（重定位表数据的）地址(RVA)
                self->relplt = (ElfW(Addr))(self->bias_addr + data->d_un.d_ptr);
                break;
            }
            case DT_PLTRELSZ:
                //.rel.plt节表项的节区数据的大小
                self->relplt_sz = data->d_un.d_val;
                break;
            case DT_REL:
            case DT_RELA:{
                self->reldyn = (ElfW(Addr))(self->bias_addr + data->d_un.d_ptr);
                break;
            }
            case DT_RELSZ:
            case DT_RELASZ:
                //.rel.dyn-size，.rel.dyn节区数据的大小
                self->reldyn_sz = data->d_un.d_val;
                break;
            case DT_ANDROID_REL:
            case DT_ANDROID_RELA:{
                self->relandroid = (ElfW(Addr))(self->bias_addr + data->d_un.d_ptr);
                break;
            }
            case DT_ANDROID_RELSZ:
            case DT_ANDROID_RELASZ:
                self->relandroid_sz = data->d_un.d_val;
                break;
            case DT_HASH:{
                //如果存在了DT_GNU_HASH则直接忽略DT_HASH
                if(1 == self->is_use_gnu_hash){
                    break;
                }
                raw = (uint32_t *)(self->bias_addr + data->d_un.d_ptr);
                self->bucket_cnt  = raw[0];
                self->chain_cnt   = raw[1];
                self->bucket      = &raw[2];
                self->chain       = &(self->bucket[self->bucket_cnt]);
                break;
            }
            case DT_GNU_HASH:{
                raw = (uint32_t *)(self->bias_addr + data->d_un.d_ptr);
                self->bucket_cnt  = raw[0];
                self->symoffset   = raw[1];
                self->bloom_sz    = raw[2];
                self->bloom_shift = raw[3];
                self->bloom       = (ElfW(Addr) *)(&raw[4]);
                self->bucket      = (uint32_t *)(&(self->bloom[self->bloom_sz]));
                self->chain       = (uint32_t *)(&(self->bucket[self->bucket_cnt]));
                self->is_use_gnu_hash = 1;
                break;
            }
            default:
                break;
        }
        data = (ElfW(Dyn *))((UPOINT)data + sizeof(ElfW(Dyn)));
    }

    if(0 != self->relandroid){
        const char *rel = (const char *)self->relandroid;
        if(self->relandroid_sz < 4 ||
           rel[0] != 'A' ||
           rel[1] != 'P' ||
           rel[2] != 'S' ||
           rel[3] != '2'){
            return 0;
        }
        self->relandroid += 4;
        self->relandroid_sz -= 4;
    }
    return 1;
}

int antiPlt::parse_mem_dynamic_segment(elf_dyn_info* self, UPOINT base) {
    ElfW(Dyn *) dyn_data = nullptr;
    ElfW(Addr) dyn_vaddr = 0;
    ElfW(Word) dyn_memsz = 0;

    self->memSoSize = get_so_size(self);
    dyn_vaddr = self->elf_dynamic->p_vaddr;
    dyn_memsz = get_dynamic_data_size(base, dyn_vaddr);
    dyn_data = (ElfW(Dyn *))(base + dyn_vaddr);
    self->bias_addr = base - self->elf_loadSegAry[0]->p_vaddr;
    if (!parse_dynamic_segment(self, dyn_data, dyn_memsz)){
        return 0;
    }
    return 1;
}

UPOINT antiPlt::rva2fa(elf_dyn_info *self, UPOINT rva){
//    if((pt_load.p_vaddr + pt_load.p_memsz) > PT_DYNAMIC.p_vaddr && PT_DYNAMIC.p_vaddr > pt_load.p_vaddr){
//        PT_DYNAMIC.p_vaddr - pt_load.p_vaddr = offset
//        pt_load.offset + offset = PT_DYNAMIC.offset
//    }
    UPOINT off = 0;
    UPOINT fa = 0;

    for (int i = 0; i < self->loadCount; ++i) {
        if((self->elf_loadSegAry[i]->p_vaddr + self->elf_loadSegAry[i]->p_memsz > rva)
           && (rva >= self->elf_loadSegAry[i]->p_vaddr)){
            off = rva - self->elf_loadSegAry[i]->p_vaddr;
            fa = off + self->elf_loadSegAry[i]->p_offset;
            break;
        }
    }
    return fa;
}

int antiPlt::parse_mmap_dynamic_segment(elf_dyn_info *self, UPOINT base) {
    ElfW(Dyn *) dyn_data = nullptr;
    ElfW(Addr) dyn_offset = 0;
    ElfW(Word) dyn_filesz = 0;

    self->memSoSize = mmapSoSize;
    //不能直接用p_offset，需要自己rva转fa
    //dyn_offset = self->elf_dynamic->p_offset;
    dyn_offset = rva2fa(self, self->elf_dynamic->p_vaddr);
    dyn_filesz = get_dynamic_data_size(base, dyn_offset);
    dyn_data = (ElfW(Dyn *))(base + dyn_offset);
    //不能直接用p_offset，需要自己rva转fa
    self->bias_addr = base - self->elf_loadSegAry[0]->p_offset;
    if (!parse_dynamic_segment(self, dyn_data, dyn_filesz)){
        return 0;
    }
    return 1;
}

int antiPlt::check_rel_plt(elf_dyn_info *self) {
    ElfW(Rela *) rela = nullptr;
    ElfW(Rel *)  rel = nullptr;
    size_t relCount = 0;
    ElfW(Sym) *sym = nullptr;

    if(0 != self->relplt){
        if (0 == self->is_use_rela){
            //使用Rel结构体解析
            rel = (ElfW(Rel *))self->relplt;
            relCount = self->relplt_sz / sizeof(ElfW(Rel));
            for (int i = 0; i <= relCount; ++i) {
                //再次确定属于.rel.plt项的数据并获取记录.rel.plt表中记录函数调用的地址
                if (ELF_R_GENERIC_JUMP_SLOT == ELF_R_TYPE(rel->r_info)){
                    //判断获取的.rel.plt中函数的地址是否超过了该so的范围,如果超过了则计算导入函数在哪个需求so的范围内，如果不符合则判定为Hook状态
                    //并且记录所有的API名保存到数组中用以跟后续检测.rel.dyn使用
                    sym = &(self->symtab[ELF_R_SYM(rel->r_info)]);
                    if (nullptr != (UPOINT *)(rel->r_offset + self->bias_addr)){
                        if (*(UPOINT *)(rel->r_offset + self->bias_addr) > (self->memSoSize + (UPOINT)self->elf_hdr) ||
                                *(UPOINT *)(rel->r_offset + self->bias_addr) < (UPOINT)self->elf_hdr){
                            //判断导入函数是否位于其所需so可执行的Segment的范围内
                            if (1 == check_mem_rel_plt_import_func_is_hooked(self, *(UPOINT *)(rel->r_offset + self->bias_addr))){
                                LOGD(".rel.plt已被Hook rel.plt第%d项 符号表第%d项 函数地址:%p, 函数名:%s",
                                     i,
                                     ELF_R_SYM(rel->r_info),
                                     *(UPOINT *)(rel->r_offset + self->bias_addr),
                                     self->strtab + sym->st_name);
                                //return 1;
                            }
                        }else{
                            //检测自身so是否通过.rel.plt表Hook了函数地址
                            //判断sym中的st_size和st_value是否为0，如果为0说明该函数为导入函数，且因已经修改了导入函数的地址，故判定为是对自身进行plt的Hook
                            //也就是说*(UPOINT *)(rel->r_offset + self->bias_addr)函数地址在自身so的范围内（那就应该不是导入函数），但在对应sym结构体上却是个导入函数，所以判定为被hook
                            if (0 == sym->st_size && 0 == sym->st_value){
                                LOGD(".rel.plt中的导入函数已被Hook 符号名为->%s, Hook的函数地址->%p",
                                     self->strtab + sym->st_name,
                                     *(UPOINT *)(rel->r_offset + self->bias_addr));
                                //return 1;
                            }else{
                                //此处为针对自身so的自定义函数是否被PLTHook的情况
                                //内存中的函数地址 - maps的so基址 对比 文件中所记录的偏移，比对失败则判定为被Hook状态
                                if (!mapping_file(memSelf)){
                                    LOGD("映射源文件失败");
                                    return 0;
                                }else{
                                    if (nullptr != elf_parse_mmap_so(mmapSelf)){
                                        //通过比对偏移判断自身的.rel.plt中的函数是否遭到Hook
                                        if (1 == check_rel_customize_sym_is_hooked(mmapSelf, self, 1,
                                                                        *(UPOINT *)(rel->r_offset + self->bias_addr),
                                                                        i)){

                                            LOGD("与源文件中记录的offset不同，故判定 .rel.plt中的导入函数已被Hook 符号名为->%s, Hook的函数地址->%p",
                                                 self->strtab + sym->st_name,
                                                 *(UPOINT *)(rel->r_offset + self->bias_addr));
                                            //return 1;
                                        }
                                    }else{
                                        LOGD("解析映射的so文件的segment失败");
                                        return 0;
                                    }
                                }
                            }
                        }
                    }
                }
                rel = (ElfW(Rel *))((UPOINT)rel + sizeof(ElfW(Rel)));
            }
        }else if(1 == self->is_use_rela){
            //使用Rela结构体解析
            rela = (ElfW(Rela *))self->relplt;
            relCount = self->relplt_sz / sizeof(ElfW(Rela));
            for (int i = 0; i <= relCount; ++i) {
                //再次确定属于.rela.plt项的数据并获取记录.rela.plt表中记录函数调用的地址
                if (ELF_R_GENERIC_JUMP_SLOT == ELF_R_TYPE(rela->r_info)){
                    //判断获取的.rela.plt中函数的地址是否超过了该so的范围,如果超过了则计算导入函数在哪个lib的范围内，如果不符合则判定为Hook状态
                    //并且记录所有的API名保存到数组中用以跟后续检测.rela.dyn使用
                    sym = &(self->symtab[ELF_R_SYM(rela->r_info)]);
                    //如果是自己对.rela.plt中的函数进行了Hook，那么也是本so的函数地址，先对其进行PASS
                    if (nullptr != (UPOINT *)(rela->r_offset + self->bias_addr)){
                        if (*(UPOINT *)(rela->r_offset + self->bias_addr) > (self->memSoSize + (UPOINT)self->elf_hdr) ||
                                *(UPOINT *)(rela->r_offset + self->bias_addr) < (UPOINT)self->elf_hdr){
                            if (1 == check_mem_rel_plt_import_func_is_hooked(self, *(UPOINT *)(rela->r_offset + self->bias_addr))){
                                LOGD(".rela.plt已被Hook rela.plt第%d项 符号表第%d项 函数地址:%p, 函数名:%s", i, ELF_R_SYM(rela->r_info),
                                     *(UPOINT *)(rela->r_offset + self->bias_addr),
                                     self->strtab + sym->st_name);
                                //return 1;
                            }
                        }else{
                            //检测自身so是否通过.rela.plt表Hook了函数地址
                            //判断sym中的st_size和st_value是否为0，如果为0说明该函数导入函数，且因已经修改了导入函数的地址，故判定为对自身进行PLT的Hook
                            if (0 == sym->st_size && 0 == sym->st_value){
                                LOGD(".rela.plt中的导入函数已被Hook 符号名为->%s, Hook的函数地址->%p",
                                     self->strtab + sym->st_name,
                                     *(UPOINT *)(rela->r_offset + self->bias_addr));
                                //return 1;
                            }else{
                                //这里是对so的自定义函数是否被PLTHook的情况
                                //内存中的函数地址 - so基址 对比 文件中所记录的偏移，比对失败则判定为被Hook状态
                                if (!mapping_file(memSelf)){
                                    LOGD("映射源文件失败");
                                    return 0;
                                }else{
                                    if (nullptr != elf_parse_mmap_so(mmapSelf)){
                                        //通过比对偏移判断自身的.rela.plt中的函数是否遭到Hook
                                        if (1 == check_rel_customize_sym_is_hooked(mmapSelf, self, 1,
                                                                        *(UPOINT *)(rela->r_offset + self->bias_addr),
                                                                        i)){
                                            LOGD("与源文件中记录的offset不同，故判定 .rela.plt中的导入函数已被Hook 符号名为->%s, Hook的函数地址->%p",
                                                 self->strtab + sym->st_name,
                                                 *(UPOINT *)(rela->r_offset + self->bias_addr));
                                            //return 1;
                                        }
                                    }else{
                                        LOGD("解析映射的so文件的segment失败");
                                        return 0;
                                    }
                                }
                            }
                        }
                    }
                }
                rela = (ElfW(Rela *))((UPOINT)rela + sizeof(ElfW(Rela)));
            }
        }
    }
    return 0;
}

//修改方向：范围大小从需要的libs的size改为libc中拥有可执行属性的Segment的大小
int antiPlt::check_mem_rel_plt_import_func_is_hooked(elf_dyn_info *self, UPOINT importAddr) {
    SOINFO *tmp = nullptr;
    elf_dyn_info *dynInfo = nullptr;

    if (nullptr == self->needLibHdr && nullptr != self){
        if (0 == get_so_needed_libs(self)){
            LOGD("self为nullptr");
            return 0;
        }
    }
    tmp = self->needLibHdr;
    while(true){
        if (nullptr == tmp){
            //遍历自身so所需要的所有模块的rw属性的segment后，该导入函数如不在所需的so的可读可执行的段范围内，判定为被Hook了
            return 1;
        }
        if (strstr(tmp->soName, memSoName)){
            //过滤自身模块名
            tmp = tmp->next;
        }else{
            dynInfo = new elf_dyn_info();
            memset(dynInfo, 0, sizeof(elf_dyn_info));
            //获取各个所需的lib在maps中的起始地址, get_maps_so_base该函数存在效率问题
            dynInfo->elf_hdr = (ElfW(Ehdr *))(UPOINT)get_maps_so_base(tmp->soName, dynInfo);
            dynInfo->is_parse_mem = 1;
            parse_segment(dynInfo, (UPOINT)get_maps_so_base(tmp->soName, dynInfo));
            //此处从so的范围改为可执行段的范围
            for (int i = 0; i < dynInfo->loadCount; ++i) {
                if (0 != (dynInfo->elf_loadSegAry[i]->p_flags & 1)){
                    //确定拥有x属性的segment
                    tmp->soBegin = dynInfo->bias_addr + dynInfo->elf_loadSegAry[i]->p_vaddr;
                    tmp->soEnd = dynInfo->bias_addr + dynInfo->elf_loadSegAry[i]->p_vaddr + dynInfo->elf_loadSegAry[i]->p_memsz;
                    if (tmp->soBegin <= importAddr && importAddr <= tmp->soEnd){
                        delete [] dynInfo->elf_loadSegAry;
                        delete dynInfo;
                        dynInfo = nullptr;
                        return 0;
                    }
                }
            }
            delete [] dynInfo->elf_loadSegAry;
            delete dynInfo;
            dynInfo = nullptr;
            tmp = tmp->next;
        }
    }
}

int antiPlt::check_rel_dyn(elf_dyn_info *self) {
    ElfW(Rela *) rela = nullptr;
    ElfW(Rel *)  rel = nullptr;
    ElfW(Sym *) sym = nullptr;
    size_t relCount = 0;

    if (0 != self->reldyn){
        if (0 == self->is_use_rela){
            //使用Rel结构体解析.rel.dyn
            rel = (ElfW(Rel *))self->reldyn;
            relCount = self->reldyn_sz / sizeof(ElfW(Rel));
            for (int i = 0; i < relCount; ++i) {
                sym = &(self->symtab[ELF_R_SYM(rel->r_info)]);
                if (ELF_R_GENERIC_GLOB_DAT == ELF_R_TYPE(rel->r_info) || ELF_R_GENERIC_ABS == ELF_R_TYPE(rel->r_info)){
                    if (nullptr != (UPOINT *)(rel->r_offset + self->bias_addr)){
                        //.rel.dyn中记录的全局变量的地址超过了自身so的范围，先判断是否在所需的lib中被定义
                        if (*(UPOINT *)(rel->r_offset + self->bias_addr) > (self->memSoSize + (UPOINT)self->elf_hdr) ||
                                *(UPOINT *)(rel->r_offset + self->bias_addr) < (UPOINT)self->elf_hdr){
                            if (1 == check_mem_rel_dyn_value_is_hooked(self, *(UPOINT *)(rel->r_offset + self->bias_addr))){
                                LOGD(".rel.dyn 第%d项已被Hook，符号表第%d项 变量地址->%p，符号名->%s",
                                     i, ELF_R_SYM(rel->r_info),
                                     *(UPOINT *)(rel->r_offset + self->bias_addr),
                                     self->strtab + sym->st_name);
                                //return 1;
                            }
                        }else{
                            if (0 == sym->st_size && 0 == sym->st_value){
                                LOGD(".rel.dyn 第%d项已被Hook 符号表第%d项 符号名为->%s, 符号地址->%p",
                                     i, ELF_R_SYM(rel->r_info),
                                     self->strtab + sym->st_name,
                                     *(UPOINT *)(rel->r_offset + self->bias_addr));
                                //return 1;
                            }else{
                                if (!mapping_file(memSelf)){
                                    LOGD("映射源文件失败");
                                    return 0;
                                }else{
                                    if (nullptr != elf_parse_mmap_so(mmapSelf)){
                                        //通过比对偏移判断自身的.rel.dyn 中的函数是否遭到Hook
                                        if (1 == check_rel_customize_sym_is_hooked(mmapSelf, self, 2,
                                                                              *(UPOINT *)(rel->r_offset + self->bias_addr),
                                                                              i)){

                                            LOGD("与源文件中记录的offset不同，故判定.rel.dyn中的符号第%d项已被Hook 符号表第%d项 符号名为->%s, 被Hook的符号地址->%p",
                                                 i, ELF_R_SYM(rel->r_info),
                                                 self->strtab + sym->st_name,
                                                 *(UPOINT *)(rel->r_offset + self->bias_addr));
                                            //return 1;
                                        }
                                    }else{
                                        LOGD("解析映射的so文件的segment失败");
                                        return 0;
                                    }
                                }
                            }
                        }
                    }
                }
                rel = (ElfW(Rel *))((UPOINT)rel + sizeof(ElfW(Rel)));
            }
        }else if (1 == self->is_use_rela){
            //使用Rela结构体解析.rela.dyn
            rela = (ElfW(Rela *))self->reldyn;
            relCount = self->reldyn_sz / sizeof(ElfW(Rela));
            for (int i = 0; i < relCount; ++i) {
                sym = &(self->symtab[ELF_R_SYM(rela->r_info)]);
                if (ELF_R_GENERIC_GLOB_DAT == ELF_R_TYPE(rela->r_info) || ELF_R_GENERIC_ABS == ELF_R_TYPE(rela->r_info)){
                    if (nullptr != (UPOINT *)(rela->r_offset + self->bias_addr)){
                        //.rela.dyn中记录的全局变量所记录的（函数指针的地址）超过了自身so的范围
                        if (*(UPOINT *)(rela->r_offset + self->bias_addr) > (self->memSoSize + (UPOINT)self->elf_hdr) ||
                                *(UPOINT *)(rela->r_offset + self->bias_addr) < (UPOINT)self->elf_hdr){
                            if (1 == check_mem_rel_dyn_value_is_hooked(self, *(UPOINT *)(rela->r_offset + self->bias_addr))){
                                LOGD(".rel.dyn 第%d项已被Hook，符号表第%d项 变量地址->%p，符号名->%s", i, ELF_R_SYM(rela->r_info),
                                     *(UPOINT *)(rela->r_offset + self->bias_addr),
                                     self->strtab + sym->st_name);
                                //return 1;
                            }
                        }else{
                            if (0 == sym->st_size && 0 == sym->st_value){
                                LOGD(".rela.dyn 第%d项已被Hook 符号表第%d项 符号名为->%s, 符号地址->%p",
                                     i, ELF_R_SYM(rela->r_info),
                                     self->strtab + sym->st_name,
                                     *(UPOINT *)(rela->r_offset + self->bias_addr));
                                //return 1;
                            }else{
                                //映射源文件进行对比
                                if (!mapping_file(memSelf)){
                                    LOGD("映射源文件失败");
                                    return 0;
                                }else{
                                    if (nullptr != elf_parse_mmap_so(mmapSelf)){
                                        //通过对比偏移判断自身的.rela.dyn是否遭到Hook
                                        if (1 == check_rel_customize_sym_is_hooked(mmapSelf, self, 2,
                                                                              *(UPOINT *)(rela->r_offset + self->bias_addr),
                                                                              i)){
                                            LOGD("与源文件中记录的offset不同，故判定.rela.dyn中的符号第%d项已被Hook 符号表第%d项 符号名为->%s, 被Hook的符号地址->%p",
                                                 i, ELF_R_SYM(rela->r_info),
                                                 self->strtab + sym->st_name,
                                                 *(UPOINT *)(rela->r_offset + self->bias_addr));
                                            //return 1;
                                        }
                                    }else{
                                        LOGD("解析映射的so文件的segment失败");
                                        return 0;
                                    }
                                }
                            }
                        }
                    }
                }
                rela = (ElfW(Rela *))((UPOINT)rela + sizeof(ElfW(Rela)));
            }
        }
    }
    return 0;
}

//修改方向：范围大小定为需要的libs的so的大小
int antiPlt::check_mem_rel_dyn_value_is_hooked(elf_dyn_info *self, UPOINT symAddr) {
    SOINFO *tmp = nullptr;
    elf_dyn_info *dynInfo = nullptr;

    if (nullptr == self->needLibHdr && nullptr != self){
        if (0 == get_so_needed_libs(self)){
            LOGD("self为nullptr");
            return 0;
        }
    }
    tmp = self->needLibHdr;
    while(true){
        if (nullptr == tmp){
            //遍历自身so所需要的所有模块后，该导入函数如还是不在所需的so的范围内，判定为被Hook了
            return 1;
        }
        if (strstr(tmp->soName, memSoName)){
            //过滤自身模块名
            tmp = tmp->next;
        }else{
            dynInfo = new elf_dyn_info();
            memset(dynInfo, 0, sizeof(elf_dyn_info));
            //获取各个所需的lib在maps中的起始地址
            tmp->soBegin = (UPOINT)get_maps_so_base(tmp->soName, dynInfo);
            dynInfo->elf_hdr = (ElfW(Ehdr *))(UPOINT)get_maps_so_base(tmp->soName, dynInfo);
            dynInfo->is_parse_mem = 1;
            parse_segment(dynInfo, tmp->soBegin);
            tmp->soEnd = get_so_size(dynInfo) + dynInfo->bias_addr;
            if (tmp->soBegin <= symAddr && symAddr <= tmp->soEnd){
                if (nullptr != dynInfo->elf_loadSegAry){
                    delete [] dynInfo->elf_loadSegAry;
                    dynInfo->elf_loadSegAry = nullptr;
                }
                if (nullptr != dynInfo){
                    delete dynInfo;
                    dynInfo = nullptr;
                }
                return 0;
            }
            if (nullptr != dynInfo->elf_loadSegAry){
                delete [] dynInfo->elf_loadSegAry;
                dynInfo->elf_loadSegAry = nullptr;
            }
            if (nullptr != dynInfo){
                delete dynInfo;
                dynInfo = nullptr;
            }
            tmp = tmp->next;
        }
    }
}

int antiPlt::check_rel_android(elf_dyn_info *self) {
    if (0 == self->relandroid || 0 == self->relandroid_sz){
        LOGD("DT_ANDROID_REL or DT_ANDROID_RELA is nullptr");
        return 0;
    }

    LOGD("暂不支持 DT_ANDROID_REL or DT_ANDROID_RELA");
    return 0;
}

int antiPlt::check_rel_customize_sym_is_hooked(elf_dyn_info *mmap, elf_dyn_info *mem, int plt_or_dyn, UPOINT symAddr, size_t num) {
    ElfW(Addr) symOffset = 0;
    ElfW(Rela *) rela = nullptr;
    ElfW(Rel *) rel = nullptr;
    ElfW(Sym *) sym = nullptr;

    symOffset = symAddr - mem->bias_addr;
    if (mem->is_use_rela){
        if (1 == plt_or_dyn){
            rela = (ElfW(Rela *))((UPOINT)mmap->relplt + sizeof(ElfW(Rela)) * num);
        }
        else if (2 == plt_or_dyn){
            rela = (ElfW(Rela *))((UPOINT)mmap->reldyn + sizeof(ElfW(Rela)) * num);
        }
        sym = &mmap->symtab[ELF_R_SYM(rela->r_info)];
        if (sym->st_value == symOffset){
            return 0;
        }else{
            if (2 == plt_or_dyn){
                //判断是否是this指针的情况，如果是类符号有多重继承的情况，那么从sym->st_value获取的要比值要比symOffset少一个8（32位）或者16字节（64位）
                if((sym->st_value + MULTIPLE_INHERITANCE_SIZE) == symOffset){
                    return 0;
                }else{
                    return 1;
                }
            }
            return 1;
        }
    }else{
        if (1 == plt_or_dyn){
            rela = (ElfW(Rela *))((UPOINT)mmap->relplt + sizeof(ElfW(Rela)) * num);
        }
        else if (2 == plt_or_dyn){
            rela = (ElfW(Rela *))((UPOINT)mmap->reldyn + sizeof(ElfW(Rela)) * num);
        }
        sym = &mmap->symtab[ELF_R_SYM(rel->r_info)];
        if (sym->st_value == symOffset){
            return 0;
        }else{
            if (2 == plt_or_dyn){
                //判断是否是this指针的情况，如果是类符号有多重继承的情况，那么从sym->st_value获取的要比值要比symOffset少一个8（32位）或者16字节（64位）
                if((sym->st_value + MULTIPLE_INHERITANCE_SIZE) == symOffset){
                    return 0;
                }else{
                    return 1;
                }
            }
            return 1;
        }
    }
}

//可能会存在未将linker或linker64拉入需要的Libs中
int antiPlt::get_so_needed_libs(elf_dyn_info *self) {
    UPOINT strBegin = 0;
    size_t strsz = 0;
    UPOINT strEnd = 0;
    size_t soNameLen = 0;
    char *c = nullptr;
    char *libName = nullptr;

    if (nullptr == self){
        return 0;
    }
    strBegin = (UPOINT)self->strtab;
    strsz = self->strsz;
    strEnd = (strsz + (UPOINT)strBegin);

    for (size_t i = strEnd; i >= strBegin; i--) {
        u_char *tmp = (u_char *)(i);
        if (0x6f == *tmp && 0x73 == *(u_char *)((UPOINT)tmp - 1) && 0x2e == *(u_char *)((UPOINT)tmp - 2)){
            c = (char *)tmp;
            while(true){
                if (*c == 0){
                    //将lib名保存到链表中
                    if (nullptr == self->needLibHdr){
                        self->needLibHdr = new SOINFO();
                        memset(self->needLibHdr, 0, sizeof(SOINFO));
                        strncpy((self->needLibHdr->soName), c + 1, soNameLen);
                    }else{
                        SOINFO *tmp = self->needLibHdr;
                        while(true){
                            if (nullptr == tmp->next){
                                break;
                            }
                            tmp = tmp->next;
                        }
                        tmp->next = new SOINFO ();
                        memset(tmp->next, 0, sizeof(SOINFO));
                        strncpy((tmp->next->soName), c + 1, soNameLen);
                    }
                    i -= soNameLen;
                    soNameLen = 0;
                    break;
                }
                soNameLen++;
                (UPOINT)c--;
            }
        }
    }
    return 1;
}

//计算了包括so的 memsz > filesz 时的匿名空间
size_t antiPlt::get_so_size(elf_dyn_info *self) {
    //当最后一个段的memsz大于filesz，那么超过的部分则会使用匿名申请内容空间
    UPOINT num1 = self->elf_loadSegAry[self->loadCount - 1]->p_memsz -
                    (self->elf_loadSegAry[self->loadCount - 1]->p_memsz & 0xfffff000);
    UPOINT num2 = self->elf_loadSegAry[self->loadCount - 1]->p_vaddr -
                    (self->elf_loadSegAry[self->loadCount - 1]->p_vaddr & 0xfffff000);
    if ((num2 + num1) > self->elf_loadSegAry[self->loadCount - 1]->p_align){
        //跨页
        return (self->elf_loadSegAry[self->loadCount - 1]->p_vaddr & 0xfffff000) +
                    (self->elf_loadSegAry[self->loadCount - 1]->p_memsz & 0xfffff000) +
                    (self->elf_loadSegAry[self->loadCount - 1]->p_align * 2);
    }
    else{
        if (num1 == 0 && num2 == 0){
            //极端情况
            return (self->elf_loadSegAry[self->loadCount - 1]->p_vaddr & 0xfffff000) +
                        (self->elf_loadSegAry[self->loadCount - 1]->p_memsz & 0xfffff000);
        }else{
            //没跨页
            return (self->elf_loadSegAry[self->loadCount - 1]->p_vaddr & 0xfffff000) +
                        (self->elf_loadSegAry[self->loadCount - 1]->p_memsz & 0xfffff000) +
                        (self->elf_loadSegAry[self->loadCount - 1]->p_align * 1);
        }
    }
}

//映射源文件到内存中
int antiPlt::mapping_file(elf_dyn_info *self) {
    int fd = 0;
    struct stat sb;
    void *soBase = nullptr;

    if (nullptr == mmapSelf->elf_hdr){
        if (0 != strlen(self->realSoPath)){
            self->realSoPath[strlen(self->realSoPath) - 1] = 0;
            fd = open(self->realSoPath, O_RDONLY);
            if (fd == -1) {
                LOGD("打开so失败");
                return 0;
            }

            if (fstat(fd, &sb) == -1) {
                LOGD("查询so文件信息失败");
                return 0;
            }

            mmapSoSize = sb.st_size;
            soBase = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
            if (soBase == MAP_FAILED) {
                LOGD("映射so失败");
                return 0;
            }
            mmapSelf->elf_hdr = (ElfW(Ehdr *))soBase;
            return 1;
        }
        LOGD("获取so的真实路径失败");
    }else if (nullptr != mmapSelf->elf_hdr){
        return 1;
    }
    return 0;
}

static uint32_t elf_hash(const uint8_t *name){
    uint32_t h = 0, g;
    while (*name) {
        h = (h << 4) + *name++;
        g = h & 0xf0000000;
        h ^= g;
        h ^= g >> 24;
    }
    return h;
}

int antiPlt::elf_hash_lookup(elf_dyn_info *self, const char *symName, uint32_t *symIdx) {
    const char *symbol_cur = nullptr;
    uint32_t i = 0;
    uint32_t hash = 0;

    hash =elf_hash((uint8_t *)symName);
    for(i = self->bucket[hash % self->bucket_cnt]; 0 != i; i = self->chain[i]){
        symbol_cur = self->strtab + self->symtab[i].st_name;
        if(0 == strcmp(symName, symbol_cur)){
            *symIdx = i;
            return 1;
        }
    }
    return 0;
}

static uint32_t elf_gnu_hash(const uint8_t *name){
    uint32_t h = 5381;
    while(*name != 0){
        h += (h << 5) + *name++;
    }
    return h;
}

int antiPlt::elf_gnu_hash_lookup_def(elf_dyn_info *self, const char *symbol, uint32_t *symIdx){
    uint32_t hash = elf_gnu_hash((uint8_t *)symbol);

    static uint32_t elfclass_bits = sizeof(ElfW(Addr)) * 8;
    size_t word = self->bloom[(hash / elfclass_bits) % self->bloom_sz];
    size_t mask = 0
                  | (size_t)1 << (hash % elfclass_bits)
                  | (size_t)1 << ((hash >> self->bloom_shift) % elfclass_bits);

    //if at least one bit is not set, this symbol is surely missing
    if((word & mask) != mask){
        return 0;
    }

    //ignore STN_UNDEF
    uint32_t i = self->bucket[hash % self->bucket_cnt];
    if(i < self->symoffset){
        return 0;
    }

    //loop through the chain
    while(1){
        const char *symname = self->strtab + self->symtab[i].st_name;
        const uint32_t symhash = self->chain[i - self->symoffset];

        if((hash | (uint32_t)1) == (symhash | (uint32_t)1) && 0 == strcmp(symbol, symname)){
            *symIdx = i;
            return 1;
        }

        //chain ends with an element with the lowest bit set to 1
        if(symhash & (uint32_t)1) break;

        i++;
    }

    return 0;
}

int antiPlt::elf_gnu_hash_lookup_undef(elf_dyn_info *self, const char *symbol, uint32_t *symIdx){
    for(uint32_t i = 0; i < self->symoffset; i++){
        const char *symname = self->strtab + self->symtab[i].st_name;
        if(0 == strcmp(symname, symbol)){
            *symIdx = i;
            return 1;
        }
    }
    return 0;
}

int antiPlt::elf_gnu_hash_lookup(elf_dyn_info *self, const char *symName, uint32_t *symIdx) {
    if(0 == elf_gnu_hash_lookup_def(self, symName, symIdx)){
        return 1;
    }
    if(0 == elf_gnu_hash_lookup_undef(self, symName, symIdx)) {
        return 1;
    }
    return 0;
}

int antiPlt::elf_find_symidx_by_name(elf_dyn_info *self, const char *symbol, uint32_t *symIdx){
    if(self->is_use_gnu_hash)
        return elf_gnu_hash_lookup(self, symbol, symIdx);
    else
        return elf_hash_lookup(self, symbol, symIdx);
}