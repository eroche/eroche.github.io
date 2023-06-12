// tkz3_builder.c

#include"lang.h"

void set_fst_for_node_tkz3(LR1_TREE_NODE node,FST_O fst);

#pragma mark - TKZ3_BUILDER_VAR

typedef struct _tkz3_builder_var{
    
    char *varname;
    FST_O fst;
    
    
} *TKZ3_BUILDER_VAR;

TKZ3_BUILDER_VAR init_tkz3_builder_var(void){
    TKZ3_BUILDER_VAR var=(TKZ3_BUILDER_VAR)L_ALLOC(sizeof(struct _tkz3_builder_var));
    memset(var,0,sizeof(struct _tkz3_builder_var));
    
    return var;
}

void free_tkz3_builder_var(TKZ3_BUILDER_VAR var){
    
    if (var->varname)
        L_FREE(var->varname);
    
    if (var->fst)
        free_fst_o(var->fst);
    
    L_FREE(var);
}

#pragma mark - TKZ3_BUILDER_TOKEN_TYPE

typedef struct _tkz3_builder_token_type{

    unsigned long weight;
    
    TKZ3_BUILDER_VAR var;
    
} *TKZ3_BUILDER_TOKEN_TYPE;

TKZ3_BUILDER_TOKEN_TYPE init_tkz3_builder_token_type(void){
    TKZ3_BUILDER_TOKEN_TYPE token_type=(TKZ3_BUILDER_TOKEN_TYPE)L_ALLOC(sizeof(struct _tkz3_builder_token_type));
    memset(token_type,0,sizeof(struct _tkz3_builder_token_type));
    
    return token_type;
}

void free_tkz3_builder_token_type(TKZ3_BUILDER_TOKEN_TYPE token_type){
    
    // WARNING: should not free the <var> because
    // It's freed directly by the builder
    
    L_FREE(token_type);
}

#pragma mark - TKZ3_BUILDER_TAG_SET

typedef struct _tkz3_builder_tag_set{
    
    unsigned long nb_tags;
    unsigned long *tag_ids;
    
} *TKZ3_BUILDER_TAG_SET;

TKZ3_BUILDER_TAG_SET init_tkz3_builder_tag_set(void){
    TKZ3_BUILDER_TAG_SET tag_set=(TKZ3_BUILDER_TAG_SET)L_ALLOC(sizeof(struct _tkz3_builder_tag_set));
    memset(tag_set,0,sizeof(struct _tkz3_builder_tag_set));
    
    return tag_set;
}

void free_tkz3_builder_tag_set(TKZ3_BUILDER_TAG_SET tag_set){
    
    if (tag_set->tag_ids)
        L_FREE(tag_set->tag_ids);
    
    L_FREE(tag_set);
}
#pragma mark - TKZ3_BUILDER

typedef struct _tkz3_builder{
    
    unsigned long rcount;
    
    // -- Reference directory (where to look for files containing list of words
    
    char *ref_directory;
    
    // -- Variables
    
    unsigned long nb_vars;
    DYN_POINTER_ARRAY vars;
    
    // -- Token types
    
    unsigned long nb_token_types;
    DYN_POINTER_ARRAY token_types;
    
    // -- Tag types
    
    unsigned long nb_tag_token_types;
    DYN_POINTER_ARRAY tag_token_types;
    
    // -- Tag Sets
    
    unsigned long nb_tag_sets;
    DYN_POINTER_ARRAY tag_sets;
    
    // Sentences
    
    FST_O end_of_sentence_fst;
    
    // ##### PARSING TKZ3
    
    LR1_BUFFALO lr1;
    LR1_RESULT lr1_result;
    LR1_ENGINE lr1_engine;

    
} *TKZ3_BUILDER;


TKZ3_BUILDER init_tkz3_builder(void){
    
    TKZ3_BUILDER builder=(TKZ3_BUILDER)L_ALLOC(sizeof(struct _tkz3_builder));
    memset(builder,0,sizeof(struct _tkz3_builder));
    
    builder->vars=init_with_size_dyn_pointer_array(100);
    
    builder->token_types=init_with_size_dyn_pointer_array(100);
    builder->tag_token_types=init_with_size_dyn_pointer_array(100);
    builder->tag_sets=init_with_size_dyn_pointer_array(100);

    builder->lr1=get_shared_lr1_buffalo_tkz3();
//    builder->lr1=build_lr1_buffalo_tkz3();
    
    builder->lr1_result=init_lr1_result();
    builder->lr1_engine=init_lr1_engine(builder->lr1);

    
    return builder;
}

void free_tkz3_builder(TKZ3_BUILDER builder){
    
    if (builder->ref_directory)
        L_FREE(builder->ref_directory);
    
    for(unsigned long i=0;i<builder->nb_vars;i++){
        
        TKZ3_BUILDER_VAR var=get_at_dyn_pointer_array(builder->vars, i);
        
        free_tkz3_builder_var(var);
        
    }
    free_dyn_pointer_array(builder->vars);
    
    for(unsigned long i=0;i<builder->nb_token_types;i++){
        TKZ3_BUILDER_TOKEN_TYPE token_type=get_at_dyn_pointer_array(builder->token_types, i);
        free_tkz3_builder_token_type(token_type);
    }
    free_dyn_pointer_array(builder->token_types);
    
    for(unsigned long i=0;i<builder->nb_tag_token_types;i++){
        TKZ3_BUILDER_TOKEN_TYPE token_type=get_at_dyn_pointer_array(builder->tag_token_types, i);
        free_tkz3_builder_token_type(token_type);
    }
    free_dyn_pointer_array(builder->tag_token_types);
    
    for(unsigned long i=0;i<builder->nb_tag_sets;i++){
        TKZ3_BUILDER_TAG_SET tag_set=get_at_dyn_pointer_array(builder->tag_sets, i);
        free_tkz3_builder_tag_set(tag_set);
    }
    free_dyn_pointer_array(builder->tag_sets);
    
    if (builder->end_of_sentence_fst)
        free_fst_o(builder->end_of_sentence_fst);

    
    free_lr1_engine(builder->lr1_engine);
    free_lr1_result(builder->lr1_result);
//    free_lr1_buffalo(builder->lr1);  // This is being freed as part of the shared data freeing

    L_FREE(builder);
}

// Creates a variable and assigns it
void set_variable_fst_tkz3_builder(TKZ3_BUILDER builder,
                                   char *variable_name,
                                   FST_O fst){
    if (!builder || !variable_name || !fst)
        return;
    
    if (!strlen(variable_name))
        return;
    
    TKZ3_BUILDER_VAR var=init_tkz3_builder_var();
    
    var->varname=(char *)L_ALLOC(sizeof(char)*(strlen(variable_name)+1));
    strcpy(var->varname,variable_name);
    
    var->fst=fst;
    fst->rcount++;
    
    set_at_dyn_pointer_array(builder->vars, builder->nb_vars++, var);
    
}

TKZ3_BUILDER_VAR get_var_with_name_tkz3_builder(TKZ3_BUILDER builder,
                                                char *varname){
    if (!builder || !builder->nb_vars)
        return NULL;
    
    TKZ3_BUILDER_VAR var=NULL;
    
    for(unsigned long i=0;i<builder->nb_vars;i++){
        TKZ3_BUILDER_VAR tmp_var=get_at_dyn_pointer_array(builder->vars, i);
        if (!strcmp(tmp_var->varname,varname)){
            var=tmp_var;
            break;
        }
    }
    
    return var;
}

// Adds a new token type with the name <token_type_string>
// WARNING: there can only be one token_type with a given name
int add_token_type_tkz3_builder(TKZ3_BUILDER builder,
                                TKZ3_BUILDER_VAR var,
                                unsigned long weight){
    
    // Checks that the token_type doesn't already exist
    
    int found=0;
    for(unsigned long i=0;i<builder->nb_token_types;i++){
        TKZ3_BUILDER_TOKEN_TYPE token_type=get_at_dyn_pointer_array(builder->token_types, i);
        if (!strcmp(token_type->var->varname,var->varname)){
            found=1;
            break;
        }
    }
    
    if (found)
        return 0;
    
    TKZ3_BUILDER_TOKEN_TYPE token_type=init_tkz3_builder_token_type();
    token_type->var=var;
    token_type->weight=weight;
    
    set_at_dyn_pointer_array(builder->token_types, builder->nb_token_types++, token_type);
    
    return 1;
}

//
unsigned long add_tag_set_if_absent_tkz3_builder(TKZ3_BUILDER builder,
                                                 unsigned long nb_tags,
                                                 unsigned long *tag_ids){
    
    if (!nb_tags || !tag_ids || !builder)
        return 0;
    
    unsigned long tag_set_id=0;
    
    // -- First check that the tag sets doesn't already exist
    
    int found=0;
    for(unsigned long i=0;i<builder->nb_tag_sets;i++){
        TKZ3_BUILDER_TAG_SET tag_set=get_at_dyn_pointer_array(builder->tag_sets, i);
        if (tag_set->nb_tags!=nb_tags)
            continue;
        if (!memcmp(tag_ids, tag_set->tag_ids, sizeof(long)*nb_tags)){
            found=1;
            tag_set_id=i;
            break;
        }
    }
    
    if (found)
        return tag_set_id;
    
    tag_set_id=builder->nb_tag_sets++;
    
    TKZ3_BUILDER_TAG_SET tag_set=init_tkz3_builder_tag_set();
    tag_set->tag_ids=(unsigned long *)L_ALLOC(sizeof(long)*nb_tags);
    
    tag_set->nb_tags=nb_tags;
    
    memcpy(tag_set->tag_ids,tag_ids,sizeof(long)*nb_tags);
    
    set_at_dyn_pointer_array(builder->tag_sets, tag_set_id, tag_set);
    
    return tag_set_id;
    
}

#pragma mark - TOP LEVEL

TKZ3 build_from_dsa_tkz3(DYN_STRING_ARRAY dsa,unsigned long nb_lines,char *ref_directory);
int compile_line_tkz3_builder(TKZ3_BUILDER builder,char *line);

DYN_STRING_ARRAY load_tkz3_file_into_string_array(char *filename,unsigned long *p_nb_lines){
    if (!filename)
        return NULL;
    
    FILE *f=fopen(filename, "rb");
    
    if (!f){
        fprintf(stdout, "Unable to open %s.\n",filename);
        return NULL;
    }
    
    DYN_STRING_ARRAY dsa=init_with_size_dyn_string_array(100);
    
    char buff[1024];
    unsigned long nb=0;
    
    while(fgets(buff, 1000, f)){
        
        if (*buff=='\0' || *buff=='\n' || *buff=='#')
            continue;
        
        chomp_line(buff);
        
        set_at_dyn_string_array(dsa, nb++, (unsigned char *)buff, 0);
    }
    
    fclose(f);
    
    *p_nb_lines=nb;
    
    return dsa;
    
}


TKZ3 build_tkz3(unsigned char *tkz3_config_filename,char *ref_directory){
    
    if (!tkz3_config_filename)
        return NULL;
    
    unsigned long nb_lines=0;
    DYN_STRING_ARRAY dsa=load_tkz3_file_into_string_array((char *)tkz3_config_filename, &nb_lines);
    
    if (!dsa)
        return NULL;
    
    if (!nb_lines){
        free_dyn_string_array(dsa);
        return NULL;
    }
    
    TKZ3 tkz3=build_from_dsa_tkz3(dsa,nb_lines,ref_directory);
    
    free_dyn_string_array(dsa);
    
    return tkz3;

}

int compare_token_types_for_qsort(const void *t1,const void *t2){
    
    TKZ3_BUILDER_TOKEN_TYPE token_type1=*((TKZ3_BUILDER_TOKEN_TYPE *)t1);
    TKZ3_BUILDER_TOKEN_TYPE token_type2=*((TKZ3_BUILDER_TOKEN_TYPE *)t2);

    if (token_type1->weight > token_type2->weight){
        return -1;
    }
    else if (token_type1->weight < token_type2->weight)
        return 1;
    else
        return 0;
    
    return 0;
}

// This is the callback function for the union
// If two terminal states with two different state value are merged, it means
// they are pointing to two different set IDs. This function
// takes the two state IDs and creates a new tag set representing the union
// of these two sets.
// Returns the ID of tag_set_id1 UNION tag_set_id2
unsigned long create_tag_sets_for_union_callback(unsigned long tag_set_id1_plus_one,
                                                 unsigned long tag_set_id2_plus_one,
                                                 void *callback_data){
    
    TKZ3_BUILDER builder=(TKZ3_BUILDER)callback_data;
    
    TKZ3_BUILDER_TAG_SET tag_set1=get_at_dyn_pointer_array(builder->tag_sets, tag_set_id1_plus_one-1);
    TKZ3_BUILDER_TAG_SET tag_set2=get_at_dyn_pointer_array(builder->tag_sets, tag_set_id2_plus_one-1);
    
    unsigned long *tags1=tag_set1->tag_ids;
    unsigned long *tags2=tag_set2->tag_ids;
    
    unsigned long new_tags_buff[100];
    unsigned long *new_tags=new_tags_buff;
    
    if (tag_set1->nb_tags+tag_set2->nb_tags > 100){
        new_tags=(unsigned long *)L_ALLOC(sizeof(long)* (tag_set1->nb_tags + tag_set2->nb_tags));
    }
    
    // merges the two tag sets into new_tags
    
    unsigned long i1=0; // index inti <tags1>
    unsigned long i2=0; // index into <tags2>
    unsigned long i=0; // index into <new_tags>
    
    while(i1<tag_set1->nb_tags && i2<tag_set2->nb_tags){
        if (tags1[i1] < tags2[i2]){
            new_tags[i++]=tags1[i1++];
        }
        else if (tags1[i1]> tags2[i2]){
            new_tags[i++]=tags2[i2++];
        }
        else if (tags1[i1]==tags2[i2]){
            new_tags[i++]=tags1[i1];
            i1++;i2++;
        }
    }
    while(i1<tag_set1->nb_tags){
        new_tags[i++]=tags1[i1++];
    }
    while(i2<tag_set2->nb_tags){
        new_tags[i++]=tags2[i2++];
    }
    // at this point all the tags are in new_tags and they are sorted
    
    unsigned long new_nb_tags=i;
    
    // -- First check that the tag sets doesn't already exist
    
    unsigned long tag_set_id=add_tag_set_if_absent_tkz3_builder(builder, new_nb_tags, new_tags);
    
    // ### END: Frees working memory

    if (tag_set1->nb_tags+tag_set2->nb_tags > 100){
        L_FREE(new_tags);
    }
    
    return tag_set_id+1;  // +1 is the type of the terminal state

}


TKZ3 build_from_dsa_tkz3(DYN_STRING_ARRAY dsa,unsigned long nb_lines,char *ref_directory){
    
    if (!dsa || !nb_lines)
        return NULL;
    
    TKZ3_BUILDER builder=init_tkz3_builder();
    if (ref_directory)
        builder->ref_directory=ref_directory;
    
    // ###### 1. Compiles each line and adds the results to <builder>
    
    for(unsigned long i=0;i<nb_lines;i++){
        
        char *line=(char *)get_at_dyn_string_array(dsa, i, NULL);
        
        if (*line=='#' || *line=='\0')
            continue;
        
        if (!compile_line_tkz3_builder(builder,line)){
            break;
        }
        
    }
    
    // ##### 2. Merges all the token_types into one
    
    if (!builder->nb_token_types){
        free_tkz3_builder(builder);
        return NULL;
    }
    
    // ##### 2a0. Sorts the token types such that the union privileges the first ones in case of ambiguities
    
    
    qsort(builder->token_types->array,builder->nb_token_types,sizeof(void *),compare_token_types_for_qsort);
    
    // ##### 2a. Creates <S_STRING_ARRAY tkz3->token_types> that stores the types
    
    DYN_STRING_ARRAY ds_type_names=init_with_size_dyn_string_array(builder->nb_token_types+1);
    for(unsigned long i=0;i<builder->nb_token_types;i++){
        TKZ3_BUILDER_TOKEN_TYPE token_type_builder=get_at_dyn_pointer_array(builder->token_types, i);
        add_string_dyn_string_array(ds_type_names, (unsigned char *)token_type_builder->var->varname, 0);
    }
    // Adds an extra type for tags
    unsigned long state_type_for_tags=builder->nb_token_types+1;

    add_string_dyn_string_array(ds_type_names, (unsigned char *)"_TAG_",0);

    S_STRING_ARRAY s_token_types=compile_dyn_string_array_into_s_string_array(ds_type_names);
    
    free_dyn_string_array(ds_type_names);
    
    // #### 2b. Updates the state types of every FST to token_type_ID + 1
    
    for(unsigned long i=0;i<builder->nb_token_types;i++){
        
        TKZ3_BUILDER_TOKEN_TYPE token_type_builder=get_at_dyn_pointer_array(builder->token_types, i);
        
        FST_O fst=token_type_builder->var->fst;
        
        set_terminal_type_to_type_fst_o(fst, i+1);

    }
    
    // ### 2c. Builds the union if more than one FST
    
    FST_O fst_union=NULL;
    
    TKZ3_BUILDER_TOKEN_TYPE token_type_builder=get_at_dyn_pointer_array(builder->token_types, 0);
    fst_union=token_type_builder->var->fst;
    
    for(unsigned long i=1;i<builder->nb_token_types;i++){
        
        TKZ3_BUILDER_TOKEN_TYPE token_type_builder=get_at_dyn_pointer_array(builder->token_types, i);
        FST_O fst=token_type_builder->var->fst;
        
        FST_O fst2=union_fst_o(NULL, fst_union, fst);
        
        if (i>1)
            free_fst_o(fst_union);
        
        fst_union=fst2;
        
    }
    
    // -- 2c (2) - adds the TAG FSTs to the <fst_union> to make sure they are recognized as well
    // (tags that were not token will be marked as <_tag_> )
    
    S_STRING_ARRAY tag_names=NULL;
    if (builder->nb_tag_token_types){
        
        DYN_STRING_ARRAY d_tag_names=init_with_size_dyn_string_array(builder->nb_tag_token_types);
        
        for(unsigned long i=0;i<builder->nb_tag_token_types;i++){
            
            
            TKZ3_BUILDER_TOKEN_TYPE token_type_builder=get_at_dyn_pointer_array(builder->tag_token_types, i);
            
            set_at_dyn_string_array(d_tag_names, i, (unsigned char *)token_type_builder->var->varname, 0);

            FST_O fst=token_type_builder->var->fst;
            
            // updates the state types
            set_terminal_type_to_type_fst_o(fst, state_type_for_tags);
            
            FST_O fst2=union_fst_o(NULL, fst_union, fst);

            if (builder->nb_token_types>1 || i>=1)
                free_fst_o(fst_union);
            
            fst_union=fst2;
        }
        
        tag_names=compile_dyn_string_array_into_s_string_array(d_tag_names);
        
        free_dyn_string_array(d_tag_names);
    }
    
    
    // ### 2d. Computes start_token_character
    
    unsigned char *start_token_character=(unsigned char *)L_ALLOC(sizeof(char)*256);
    memset(start_token_character,0,sizeof(char)*256);
    
    FST_O_TRANS trans=init_trans_FST_O(fst_union);
    
    for(int t=first_trans_FST_O(fst_union, trans, 0);t;t=next_trans_FST_O(fst_union, trans)){
        
        unsigned char c=(unsigned char)trans->ilabel;
        
        start_token_character[c]=1;
    }
    
    free_fst_o_trans(trans);
    
    // ### 2e. Optimizes the FST into AUTDD14
    
    AUTDD14 aut_direct=build_from_fst_o_autdd14(fst_union);
    
    if (builder->nb_token_types>1 || builder->nb_tag_token_types)
        free_fst_o(fst_union);
    
    
    // ########################
    // ####### TAGs ##########
    // ########################
    
    AUTDD14 aut_direct_for_tags=NULL;
    unsigned char *start_token_character_for_tags=NULL;
    if (builder->nb_tag_token_types){
        
        // Each terminal state of the union will point to a set of tags
        
        // -------- A. Creates a TAG SET for each individual tag and updates the tag FSTs accordingly
        
        unsigned long *tag_ids=(unsigned long *)L_ALLOC(sizeof(long)*builder->nb_tag_token_types);
        memset(tag_ids,0,sizeof(long)*builder->nb_tag_token_types);
        
        for(unsigned long i=0;i<builder->nb_tag_token_types;i++){
            
            tag_ids[0]=i;
            
            unsigned long tag_set_id=add_tag_set_if_absent_tkz3_builder(builder, 1, tag_ids);
            
            TKZ3_BUILDER_TOKEN_TYPE token_type_builder=get_at_dyn_pointer_array(builder->tag_token_types, i);
            FST_O fst=token_type_builder->var->fst;
            
            // updates the state types
            set_terminal_type_to_type_fst_o(fst, tag_set_id+1);
            
        }

        // -------- C. Unions of all the FSTs
        
        token_type_builder=get_at_dyn_pointer_array(builder->tag_token_types, 0);
        FST_O fst_union_for_tag_sets=token_type_builder->var->fst;
        
        for(unsigned long i=1;i<builder->nb_tag_token_types;i++){
            
            TKZ3_BUILDER_TOKEN_TYPE token_type_builder=get_at_dyn_pointer_array(builder->tag_token_types, i);
            FST_O fst=token_type_builder->var->fst;
            
            int has_ambiguity=0;
            
            FST_O fst2=union_with_ambiguity_fst_o(NULL,
                                                  fst_union_for_tag_sets,
                                                  fst,
                                                  &has_ambiguity,
                                                  art_use_callback,
                                                  &create_tag_sets_for_union_callback,
                                                  (void *)builder);
            
            if (i>1)
                free_fst_o(fst_union_for_tag_sets);
            
            fst_union_for_tag_sets=fst2;
            
        }
        
        L_FREE(tag_ids);
        
        // -------- D. Unions of all the FSTs
        
        start_token_character_for_tags=(unsigned char *)L_ALLOC(sizeof(char)*256);
        memset(start_token_character_for_tags,0,sizeof(char)*256);
        
        FST_O_TRANS trans=init_trans_FST_O(fst_union_for_tag_sets);
        
        for(int t=first_trans_FST_O(fst_union_for_tag_sets, trans, 0);t;t=next_trans_FST_O(fst_union_for_tag_sets, trans)){
            
            unsigned char c=(unsigned char)trans->ilabel;
            
            start_token_character_for_tags[c]=1;
        }
        
        free_fst_o_trans(trans);

        
        // -------- E. Optimizes the FST into AUTDD14 for the tag sets
        
        aut_direct_for_tags=build_from_fst_o_autdd14(fst_union_for_tag_sets);
        
        if (builder->nb_tag_token_types>1)
            free_fst_o(fst_union_for_tag_sets);
    }

    // ########################
    // #### 3. Puts everything into TKZ3
    // ########################

    TKZ3 tkz3=(TKZ3)L_ALLOC(sizeof(struct _tkz3));
    memset(tkz3,0,sizeof(struct _tkz3));
    
    tkz3->start_token_character=start_token_character;
    tkz3->aut_direct=aut_direct;
    tkz3->token_types=s_token_types;
    
    tkz3->state_type_for_tags=state_type_for_tags;
    
    tkz3->config=compile_dyn_string_array_into_s_string_array(dsa);
    
    if (builder->nb_tag_sets){
        
        tkz3->tag_names=tag_names;
        
        tkz3->tag_aut_direct=aut_direct_for_tags;
        tkz3->start_tag_character=start_token_character_for_tags;
        
        flush_tag_sets_from_builder_to_tkz3(tkz3,builder);
        
    }
    
    if (builder->end_of_sentence_fst){
        unsigned char *end_of_sentence_character=(unsigned char *)L_ALLOC(sizeof(char)*256);
        memset(end_of_sentence_character,0,sizeof(char)*256);
        
        FST_O_TRANS trans=init_trans_FST_O(builder->end_of_sentence_fst);
        
        for(int t=first_trans_FST_O(builder->end_of_sentence_fst, trans, 0);
            t;
            t=next_trans_FST_O(builder->end_of_sentence_fst, trans)){
            
            unsigned char c=(unsigned char)trans->ilabel;
            
            end_of_sentence_character[c]=1;
        }
        
        free_fst_o_trans(trans);
        
        tkz3->end_of_sentence_character=end_of_sentence_character;
        
        tkz3->aut_eos=build_from_fst_o_autdd14(builder->end_of_sentence_fst);

    }

    // #### 4. Cleans up the memory
    
    free_tkz3_builder(builder);
    
    // #### 5. Sets default word types
    
    if (tkz3->default_word_type==0 &&
        tkz3->default_punc_type==0){
        
        for(unsigned long i=0;i<tkz3->token_types->nb_strings;i++){
            unsigned char *string_type=get_string_s_string_array(tkz3->token_types, i);
            if (!strcmp((char *)string_type,"word")){
                tkz3->default_word_type=i;
            }
            else if (!strcmp((char *)string_type,"punc")){
                tkz3->default_punc_type=i;
            }
            else if (!strcmp((char *)string_type,"eos")){
                tkz3->default_eos_type=i;
            }
        }
    }
    
    
    return tkz3;
}

int compile_variable_assignment_tkz3_builder(TKZ3_BUILDER builder,
                                             LR1_TREE_NODE node){
    
    
    SYMB_O symb=builder->lr1->tklex->symb;
    
    unsigned long variable_sid=string_to_label_symb_o_f(symb, "VARIABLE");
    unsigned long string_sid=string_to_label_symb_o_f(symb, "string");
    unsigned long regex_sid=string_to_label_symb_o_f(symb, "regex");
    unsigned long meta_regex_sid=string_to_label_symb_o_f(symb, "meta_regex");
    unsigned long variable_rhs_sid=string_to_label_symb_o_f(symb, "FSO_EXPRESSION");
    unsigned long assign_op_sid=string_to_label_symb_o_f(symb, "ASSIGN_OP");
    
    unsigned long equal_sid=string_to_label_symb_o_f(symb, "=");
    unsigned long plus_equal_sid=string_to_label_symb_o_f(symb, "+=");
    unsigned long minus_equal_sid=string_to_label_symb_o_f(symb, "-=");
    unsigned long and_equal_sid=string_to_label_symb_o_f(symb, "&=");

    
    if (node->nb_children==4){
        
        // The 4 arguments are VARIABLE ASSIGN_OP VARIABLE_RHS ;
        LR1_TREE_NODE node0=get_at_dyn_pointer_array(node->children,0);  // VARIABLE
        LR1_TREE_NODE node1=get_at_dyn_pointer_array(node->children,1);  // ASSIGN_OP
        LR1_TREE_NODE node2=get_at_dyn_pointer_array(node->children,2);  // FSO_EXPRESSION
        
        
        if (node0->symbol_id!=variable_sid || node1->symbol_id!=assign_op_sid || node2->symbol_id!=variable_rhs_sid)
            return 0;
        
        FST_O fst1=get_recursive_fst_lr1_tree_node(node2);
        
        if (!fst1)
            return 0;
        
        // -- Gets the variable name
        char *variable_name=NULL;
        if (node0->symbol_id==variable_sid){
            LR1_TREE_NODE node00=get_at_dyn_pointer_array(node0->children,0);
            
            if (node00->symbol_id==string_sid || node00->symbol_id==regex_sid || node00->symbol_id==meta_regex_sid)
                variable_name=(char *)node00->original_token->char_array->array;
        }
            
        // -- Gets the operator
        LR1_TREE_NODE node10=get_at_dyn_pointer_array(node1->children,0);  // VARIABLE
        
        // -- Assigns the variable
            
        if (node10->symbol_id==equal_sid){
            
            if (fst1 && variable_name){
                set_variable_fst_tkz3_builder(builder,
                                              variable_name,
                                              fst1);
            }
            else
                return 0;
        }
        else if (node10->symbol_id==plus_equal_sid){
            
            if (fst1 && variable_name){

                TKZ3_BUILDER_VAR var=get_var_with_name_tkz3_builder(builder, variable_name);
                
                if (!var || !var->fst){
                    return 0;
                }
                
                FST_O fst2=union_fst_o(NULL, var->fst, fst1);
                
                if (!fst2 || !nb_states_FST_O(fst2)){
                    if (fst2)
                        free_fst_o(fst2);
                    return 0;
                }
                
                free_fst_o(var->fst);
                var->fst=fst2;

            }
            else
                return 0;

        }
        else if (node10->symbol_id==minus_equal_sid){

            if (fst1 && variable_name){
                
                TKZ3_BUILDER_VAR var=get_var_with_name_tkz3_builder(builder, variable_name);
                
                if (!var || !var->fst){
                    return 0;
                }
                
                FST_O fst2=diff_fst_o(NULL, var->fst, fst1, 1);
                
                if (!fst2 || !nb_states_FST_O(fst2)){
                    if (fst2)
                        free_fst_o(fst2);
                    return 0;
                }
                
                free_fst_o(var->fst);
                var->fst=fst2;
                
            }
            else
                return 0;

        }
        else if (node10->symbol_id==and_equal_sid){
            
            if (fst1 && variable_name){
                
                TKZ3_BUILDER_VAR var=get_var_with_name_tkz3_builder(builder, variable_name);
                
                if (!var || !var->fst){
                    return 0;
                }
                
                FST_O fst2=inter_fst_o(NULL, var->fst, fst1, 1);
                
                if (!fst2 || !nb_states_FST_O(fst2)){
                    if (fst2)
                        free_fst_o(fst2);
                    return 0;
                }
                
                free_fst_o(var->fst);
                var->fst=fst2;
                var->fst->rcount++;
                
            }
            else
                return 0;
            
        }
         
    }
    
    return 1;
}

// Extracts all the strings from an ARG_LIST (which contains ARGs
// and other ARG lists
// (ARG_LIST (ARG  <string word>  ARG) <, ,> (ARG_LIST (ARG  <START_QUOTE ">  <string word>  <END_QUOTE ">  ARG) ARG_LIST) ARG_LIST)
unsigned long get_strings_from_arg_list_tkz3_builder(DYN_STRING_ARRAY arg_strings,
                                                     LR1_BUFFALO lr1,
                                                     LR1_TREE tree,
                                                     LR1_TREE_NODE node){
    SYMB_O symb=lr1->tklex->symb;

    unsigned long arg_list_sid=string_to_label_symb_o_f(symb, "ARG_LIST");
    unsigned long arg_sid=string_to_label_symb_o_f(symb, "ARG");
    unsigned long string_sid=string_to_label_symb_o_f(symb, "string");
    unsigned long regex_sid=string_to_label_symb_o_f(symb, "regex");
    unsigned long meta_regex_sid=string_to_label_symb_o_f(symb, "meta_regex");

    LR1_TREE_NODE *node_stack=(LR1_TREE_NODE *)L_ALLOC(sizeof(LR1_TREE_NODE)*tree->nb_nodes);
    unsigned long nb_nodes=0;
    
    unsigned long nb_strings=0;
    
    node_stack[nb_nodes++]=node;
    
    unsigned long q=0;
    
    while(q<nb_nodes){
        
        LR1_TREE_NODE current_node=node_stack[q];
        
        if (current_node->symbol_id==arg_sid){
            
            if (current_node->nb_children==1){
                // should be a string
                LR1_TREE_NODE node0=get_at_dyn_pointer_array(current_node->children, 0);
                if (node0->symbol_id==string_sid || node0->symbol_id==regex_sid || node0->symbol_id==meta_regex_sid){
                    set_at_dyn_string_array(arg_strings, nb_strings++, node0->original_token->char_array->array, 0);
                }
            }
            else if (current_node->nb_children==3){
                // (ARG  <START_QUOTE ">  <string word>  <END_QUOTE ">  ARG)
                
                LR1_TREE_NODE node0=get_at_dyn_pointer_array(current_node->children, 1);
                if (node0->symbol_id==string_sid){
                    set_at_dyn_string_array(arg_strings, nb_strings++, node0->original_token->char_array->array, 0);
                }

            }
            
        }
        else if (current_node->symbol_id==arg_list_sid){
            for(unsigned long i=0;i<current_node->nb_children;i++){
                LR1_TREE_NODE node0=get_at_dyn_pointer_array(current_node->children, i);
                node_stack[nb_nodes++]=node0;

            }
        }
        q++;
    }
    
    L_FREE(node_stack);

    return nb_strings;
}

// Compiles line like
// token(word,"word");
int compile_token_def_func_call_tkz3_builder(TKZ3_BUILDER builder,
                                             LR1_TREE_NODE node){
    
    if (node->nb_children!=4)
        return 0;
    
    /* token(word,"word"); will be parsed as:
     
     TREE (LINE (FUNCTION_CALL (FUNCTION_NAME  <string token>  FUNCTION_NAME) <( (> (ARG_LIST (ARG  <string word>  ARG) <, ,> (ARG_LIST (ARG  <START_QUOTE ">  <string word>  <END_QUOTE ">  ARG) ARG_LIST) ARG_LIST) <) )>  <; ;>  FUNCTION_CALL) LINE)
     */
    
    SYMB_O symb=builder->lr1->tklex->symb;
    
    unsigned long arg_list_sid=string_to_label_symb_o_f(symb, "ARG_LIST");

    
    // -- First gets ARG_LIST
    
    LR1_TREE_NODE node_arglist=get_at_dyn_pointer_array(node->children,2);
    
    if (node_arglist->symbol_id!=arg_list_sid)
        return 0;
    
    DYN_STRING_ARRAY dsa1=init_with_size_dyn_string_array(builder->lr1_result->tree->nb_nodes+1);
    
    unsigned long nb_arg_strings=get_strings_from_arg_list_tkz3_builder(dsa1,
                                                                        builder->lr1,
                                                                        builder->lr1_result->tree,
                                                                        node_arglist);
    
    if (nb_arg_strings!=2 && nb_arg_strings!=3){
        free_dyn_string_array(dsa1);
        return 0;
    }
    
    char *arg1_string=(char *)get_at_dyn_string_array(dsa1, 0, NULL);
    char *token_type_string=(char *)get_at_dyn_string_array(dsa1, 1, NULL);
    
    TKZ3_BUILDER_VAR var=get_var_with_name_tkz3_builder(builder, arg1_string);
    
    if (!var || !var->fst){
        free_dyn_string_array(dsa1);
        return 0;
    }
    
    if (!token_type_string || !strlen(token_type_string)){
        free_dyn_string_array(dsa1);
        return 0;
    }
    
    if (strlen(token_type_string)>MAX_TKZ3_TOKEN_TYPE_STRING){
        fprintf(stdout,
                "%s too long. Maximum token type string length is %d\n",
                token_type_string,
                MAX_TKZ3_TOKEN_TYPE_STRING);
    }
    
    unsigned long weight=0;
    if (nb_arg_strings==3){
        char *weight_string=(char *)get_at_dyn_string_array(dsa1, 2, NULL);
        weight=(unsigned long)atol((const char *)weight_string);
    }
    
    TKZ3_BUILDER_TOKEN_TYPE token_type=init_tkz3_builder_token_type();
    
    token_type->var=var;
    token_type->weight=weight;
    
    set_at_dyn_pointer_array(builder->token_types, builder->nb_token_types++, token_type);
    
    free_dyn_string_array(dsa1);

    return 1;
}

int compile_tag_def_func_call_tkz3_builder(TKZ3_BUILDER builder,
                                           LR1_TREE_NODE node){
    
    if (node->nb_children!=4)
        return 0;
    
    /* token(word,"word"); will be parsed as:
     
     TREE (LINE (FUNCTION_CALL (FUNCTION_NAME  <string tag>  FUNCTION_NAME) <( (> (ARG_LIST (ARG  <string word>  ARG) <, ,> (ARG_LIST (ARG  <START_QUOTE ">  <string word>  <END_QUOTE ">  ARG) ARG_LIST) ARG_LIST) <) )>  <; ;>  FUNCTION_CALL) LINE)
     */
    
    SYMB_O symb=builder->lr1->tklex->symb;
    
    unsigned long arg_list_sid=string_to_label_symb_o_f(symb, "ARG_LIST");
    
    
    // -- First gets ARG_LIST
    
    LR1_TREE_NODE node_arglist=get_at_dyn_pointer_array(node->children,2);
    
    if (node_arglist->symbol_id!=arg_list_sid)
        return 0;
    
    DYN_STRING_ARRAY dsa1=init_with_size_dyn_string_array(builder->lr1_result->tree->nb_nodes);
    
    unsigned long nb_arg_strings=get_strings_from_arg_list_tkz3_builder(dsa1,
                                                                        builder->lr1,
                                                                        builder->lr1_result->tree,
                                                                        node_arglist);
    
    if (nb_arg_strings!=2 && nb_arg_strings!=3){
        free_dyn_string_array(dsa1);
        return 0;
    }
    
    char *arg1_string=(char *)get_at_dyn_string_array(dsa1, 0, NULL);
    char *token_type_string=(char *)get_at_dyn_string_array(dsa1, 1, NULL);
    
    TKZ3_BUILDER_VAR var=get_var_with_name_tkz3_builder(builder, arg1_string);
    
    if (!var || !var->fst){
        fprintf(stdout, "ERROR: tkz3_builder: unknown variable %s.\n",arg1_string);
        free_dyn_string_array(dsa1);
        return 0;
    }
    
    if (!token_type_string || !strlen(token_type_string)){
        free_dyn_string_array(dsa1);
        return 0;
    }
    
    unsigned long weight=0;
    if (nb_arg_strings==3){
        char *weight_string=(char *)get_at_dyn_string_array(dsa1, 2, NULL);
        weight=(unsigned long)atol((const char *)weight_string);
    }
    
    TKZ3_BUILDER_TOKEN_TYPE token_type=init_tkz3_builder_token_type();
    
    token_type->var=var;
    token_type->weight=weight;
    
    set_at_dyn_pointer_array(builder->tag_token_types, builder->nb_tag_token_types++, token_type);
    
    free_dyn_string_array(dsa1);
    
    return 1;
}

int compile_end_of_sentence_func_call_tkz3_builder(TKZ3_BUILDER builder,
                                                   LR1_TREE_NODE node){
    
    if (node->nb_children!=4)
        return 0;
    
    SYMB_O symb=builder->lr1->tklex->symb;
    
    unsigned long arg_list_sid=string_to_label_symb_o_f(symb, "ARG_LIST");
    
    
    // -- First gets ARG_LIST
    LR1_TREE_NODE node_arglist=get_at_dyn_pointer_array(node->children,2);
    if (node_arglist->symbol_id!=arg_list_sid)
        return 0;
    DYN_STRING_ARRAY dsa1=init_with_size_dyn_string_array(builder->lr1_result->tree->nb_nodes);
    unsigned long nb_arg_strings=get_strings_from_arg_list_tkz3_builder(dsa1,
                                                                        builder->lr1,
                                                                        builder->lr1_result->tree,
                                                                        node_arglist);
    if (nb_arg_strings!=1){
        free_dyn_string_array(dsa1);
        return 0;
    }

    char *arg1_string=(char *)get_at_dyn_string_array(dsa1, 0, NULL);
    
    TKZ3_BUILDER_VAR var=get_var_with_name_tkz3_builder(builder, arg1_string);
    
    if (!var || !var->fst){
        fprintf(stdout, "ERROR: tkz3_builder: unknown variable %s.\n",arg1_string);
        free_dyn_string_array(dsa1);
        return 0;
    }
    
    builder->end_of_sentence_fst=var->fst;
    var->fst->rcount++;
    
    // ### IMPORTANT: Needs to add the eos as an extra token type
    
    TKZ3_BUILDER_TOKEN_TYPE token_type=init_tkz3_builder_token_type();
    
    token_type->var=var;
    token_type->weight=100;
    
    set_at_dyn_pointer_array(builder->token_types, builder->nb_token_types++, token_type);

    free_dyn_string_array(dsa1);

    return 1;
}


// Compiles lines like
// token(word,"word");
// or
// tag(email,"email");
int compile_function_call_tkz3_builder(TKZ3_BUILDER builder,
                                       LR1_TREE_NODE node){
    
    /* token(word,"word"); will be parsed as:
     
     TREE (LINE (FUNCTION_CALL (FUNCTION_NAME  <string token>  FUNCTION_NAME) <( (> (ARG_LIST (ARG  <string word>  ARG) <, ,> (ARG_LIST (ARG  <START_QUOTE ">  <string word>  <END_QUOTE ">  ARG) ARG_LIST) ARG_LIST) <) )>  <; ;>  FUNCTION_CALL) LINE)
     */
    
    SYMB_O symb=builder->lr1->tklex->symb;
    
    unsigned long function_name_sid=string_to_label_symb_o_f(symb, "FUNCTION_NAME");
    unsigned long string_sid=string_to_label_symb_o_f(symb, "string");
    
    char *function_name=NULL;

    LR1_TREE_NODE node0=get_at_dyn_pointer_array(node->children,0);
    
    if (node0->symbol_id!=function_name_sid)
        return 0;
    
    LR1_TREE_NODE node00=get_at_dyn_pointer_array(node0->children,0);
    
    if (node00->symbol_id!=string_sid)
        return 0;
    
    function_name=(char *)node00->original_token->char_array->array;
    
    if (!strcmp(function_name,"token")){
        return compile_token_def_func_call_tkz3_builder(builder,node);
    }
    else if (!strcmp(function_name,"tag")){
        return compile_tag_def_func_call_tkz3_builder(builder,node);
    }
    else if (!strcmp(function_name,"endOfSentence")){
        return compile_end_of_sentence_func_call_tkz3_builder(builder,node);
    }

    return 0;
}

// Compiles
// FSO_EXPRESSION
// F_ARG
// F_PLUS
// F_MINUS
// F_REGEX
int compile_fso_expression_tkz3_builder(TKZ3_BUILDER builder,char *line){
    
    SYMB_O symb=builder->lr1->tklex->symb;
    
    unsigned long f_regex_sid=string_to_label_symb_o_f(symb, "F_REGEX");
    unsigned long f_arg_sid=string_to_label_symb_o_f(symb, "F_ARG");
    unsigned long f_minus_sid=string_to_label_symb_o_f(symb, "F_MINUS");
    unsigned long f_or_list=string_to_label_symb_o_f(symb, "F_OR_LIST");
    unsigned long f_concat_sid=string_to_label_symb_o_f(symb, "F_CONCAT");
    unsigned long f_varname_sid=string_to_label_symb_o_f(symb, "VARNAME");
    unsigned long f_function_call_sid=string_to_label_symb_o_f(symb, "F_FUNCTION_CALL");
    unsigned long arg_list_sid=string_to_label_symb_o_f(symb, "ARG_LIST");
    unsigned long function_name_sid=string_to_label_symb_o_f(symb, "FUNCTION_NAME");

    unsigned long f_plus_sid=string_to_label_symb_o_f(symb, "F_PLUS");
    unsigned long f_star_sid=string_to_label_symb_o_f(symb, "F_STAR");
    unsigned long fso_expression_sid=string_to_label_symb_o_f(symb, "FSO_EXPRESSION");
    unsigned long minus_sid=string_to_label_symb_o_f(symb, "-");
    unsigned long string_sid=string_to_label_symb_o_f(symb, "string");
    unsigned long plus_sid=string_to_label_symb_o_f(symb, "+");
    unsigned long star_sid=string_to_label_symb_o_f(symb, "*");
    unsigned long or_sid=string_to_label_symb_o_f(symb, "|");
    unsigned long dot_sid=string_to_label_symb_o_f(symb, ".");
    unsigned long openp_sid=string_to_label_symb_o_f(symb, "(");
    unsigned long closep_sid=string_to_label_symb_o_f(symb, ")");


    LR1_TREE tree=builder->lr1_result->tree;
 
    int has_changed=1;
    
    while(has_changed){
        
        has_changed=0;
        
        for(unsigned long i=0;i<tree->nb_nodes;i++){
            
            LR1_TREE_NODE node=get_at_dyn_pointer_array(tree->nodes, i);
            
            if (node->is_compiled)
                continue;
            
            if (node->symbol_id==minus_sid ||
                node->symbol_id==plus_sid ||
                node->symbol_id==star_sid ||
                node->symbol_id==plus_sid ||
                node->symbol_id==dot_sid ||
                node->symbol_id==openp_sid ||
                node->symbol_id==closep_sid ||
                node->symbol_id==or_sid){
                
                // These are empty nodes
                node->is_compiled=1;
                has_changed=1;
            }
            if (node->symbol_id==f_regex_sid){
                // (F_REGEX  <START_REGEX />  <regex d>  <END_REGEX />  F_REGEX)
//                LR1_TREE_NODE node1=get_at_dyn_pointer_array(node->children, 1);
//                char *the_string=(char *)node1->original_token->char_array->array;
                
                // (F_REGEX regex F_REGEX)
                
                LR1_TREE_NODE node0=get_at_dyn_pointer_array(node->children, 0);
                char *the_string=(char *)node0->original_token->char_array->array;
                
                FST_O fst=compile_cregex(the_string,NULL);
                set_fst_for_node_tkz3(node,fst);
                
                node->is_compiled=1;
                has_changed=1;
                
                mark_completed_recursively_lr1_tree_node(node);

            }
            else if (node->symbol_id==fso_expression_sid){
                
                if (!check_all_children_compiled(node))
                    continue;
                
                if (node->nb_children==1){
                    node->is_transparent=1;
                    node->is_compiled=1;
                    has_changed=1;
                    continue;
                }
            }
            else if (node->symbol_id==f_arg_sid){
                
                if (!check_all_children_compiled(node))
                    continue;

                if (node->nb_children==1){
                    node->is_transparent=1;
                    node->is_compiled=1;
                    has_changed=1;
                    continue;
                }
                else if (node->nb_children==3){
                    LR1_TREE_NODE node0=get_at_dyn_pointer_array(node->children, 0);
                    if (node0->symbol_id==openp_sid){
                        
                        LR1_TREE_NODE node1=get_at_dyn_pointer_array(node->children, 1);
                        FST_O fst1=get_recursive_fst_lr1_tree_node(node1);
                        
                        if (!fst1)
                            return 0;
                        
                        set_fst_for_node_tkz3(node, fst1);
                        
                        node->is_compiled=1;
                        has_changed=1;
                    }
                }
            }
            else if (node->symbol_id==f_minus_sid){
                
                if (!check_all_children_compiled(node))
                    continue;
                
                if (node->nb_children!=3)
                    return 0;

                LR1_TREE_NODE node0=get_at_dyn_pointer_array(node->children, 0);
                LR1_TREE_NODE node2=get_at_dyn_pointer_array(node->children, 2);
                
                FST_O fst1=get_recursive_fst_lr1_tree_node(node0);
                FST_O fst2=get_recursive_fst_lr1_tree_node(node2);
                
                if (!fst1 || !fst2)
                    return 0;
                
                FST_O fst3=diff_fst_o(NULL, fst1, fst2, 1);
                
                set_fst_for_node_tkz3(node, fst3);

                node->is_compiled=1;
                has_changed=1;
                
                mark_completed_recursively_lr1_tree_node(node);
                
            }
            else if (node->symbol_id==f_or_list){
                
                if (!check_all_children_compiled(node))
                    continue;
                
                if (node->nb_children!=3)
                    return 0;
                
                LR1_TREE_NODE node0=get_at_dyn_pointer_array(node->children, 0);
                LR1_TREE_NODE node2=get_at_dyn_pointer_array(node->children, 2);
                
                FST_O fst1=get_recursive_fst_lr1_tree_node(node0);
                FST_O fst2=get_recursive_fst_lr1_tree_node(node2);
                
                if (!fst1 || !fst2)
                    return 0;
                
                FST_O fst3=union_fst_o(NULL, fst1, fst2);
                
                set_fst_for_node_tkz3(node, fst3);
                
                node->is_compiled=1;
                has_changed=1;
                
                mark_completed_recursively_lr1_tree_node(node);
                
            }
            else if (node->symbol_id==f_concat_sid){
                
                if (!check_all_children_compiled(node))
                    continue;
                
                if (node->nb_children!=3)
                    return 0;
                
                LR1_TREE_NODE node0=get_at_dyn_pointer_array(node->children, 0);
                LR1_TREE_NODE node2=get_at_dyn_pointer_array(node->children, 2);
                
                FST_O fst1=get_recursive_fst_lr1_tree_node(node0);
                FST_O fst2=get_recursive_fst_lr1_tree_node(node2);
                
                if (!fst1 || !fst2)
                    return 0;
                
                FST_O fsts[2];fsts[0]=fst1;fsts[1]=fst2;
                
                FST_O fst3=concat_lists_fst_o(2, fsts);
                FST_O fst4=optimize_fst_o(fst3);
                
                if (fst3!=fst4)
                    free_fst_o(fst3);
                
                set_fst_for_node_tkz3(node, fst4);
                
                node->is_compiled=1;
                has_changed=1;
                
                mark_completed_recursively_lr1_tree_node(node);
                
            }
            else if (node->symbol_id==f_plus_sid){
                
                if (!check_all_children_compiled(node))
                    continue;
                
                if (node->nb_children!=2)
                    return 0;
                
                LR1_TREE_NODE node0=get_at_dyn_pointer_array(node->children, 0);
                
                FST_O fst1=get_recursive_fst_lr1_tree_node(node0);
                
                if (!fst1)
                    return 0;
                
                FST_O fst2=plus_fst_o(fst1);
                FST_O fst3=optimize_fst_o(fst2);
                
                if (fst3!=fst2)
                    free_fst_o(fst2);
                
                set_fst_for_node_tkz3(node, fst3);
                
                node->is_compiled=1;
                has_changed=1;
                
                mark_completed_recursively_lr1_tree_node(node);
                
            }
            else if (node->symbol_id==f_star_sid){
                
                if (!check_all_children_compiled(node))
                    continue;
                
                if (node->nb_children!=2)
                    return 0;
                
                LR1_TREE_NODE node0=get_at_dyn_pointer_array(node->children, 0);
                
                FST_O fst1=get_recursive_fst_lr1_tree_node(node0);
                
                if (!fst1)
                    return 0;
                
                FST_O fst2=star_fst_o(fst1);
                FST_O fst3=optimize_fst_o(fst2);
                
                if (fst3!=fst2)
                    free_fst_o(fst2);
                
                set_fst_for_node_tkz3(node, fst3);
                
                node->is_compiled=1;
                has_changed=1;
                
                mark_completed_recursively_lr1_tree_node(node);
                
            }
            else if (node->symbol_id==f_varname_sid){
                
                if (node->nb_children!=1)
                    return 0;
                
                LR1_TREE_NODE node0=get_at_dyn_pointer_array(node->children, 0);
                
                char *variable_name=(char *)node0->original_token->char_array->array;
                
                TKZ3_BUILDER_VAR var=get_var_with_name_tkz3_builder(builder, variable_name);
                
                if (!var){
                    fprintf(stdout, "ERROR Compiling TKZ3: variable <%s> unknown.\n",variable_name);
                    return 0;
                }
                
                FST_O fst1=var->fst;
                
                if (!fst1)
                    return 0;

                set_fst_for_node_tkz3(node, fst1);

                node->is_compiled=1;
                has_changed=1;
                
                mark_completed_recursively_lr1_tree_node(node);
            }
            else if (node->symbol_id==f_function_call_sid){
                
                // F_FUNCTION_CALL : FUNCTION_NAME '(' ARG_LIST ')'
                
                node->is_compiled=1;
                has_changed=1;
                
                mark_completed_recursively_lr1_tree_node(node);
                
                // -- Gets the function name
                char *function_name=NULL;
                LR1_TREE_NODE node0=get_at_dyn_pointer_array(node->children,0);
                if (node0->symbol_id!=function_name_sid)
                    return 0;
                LR1_TREE_NODE node00=get_at_dyn_pointer_array(node0->children,0);
                if (node00->symbol_id!=string_sid)
                    return 0;
                function_name=(char *)node00->original_token->char_array->array;
                
                if (strcmp(function_name, "list")){
                    fprintf(stdout, "Unknown function name in compiling TKZ3: <%s>.\n",function_name);
                    return 0;
                }
                
                // -- First gets ARG_LIST
                LR1_TREE_NODE node_arglist=get_at_dyn_pointer_array(node->children,2);
                if (node_arglist->symbol_id!=arg_list_sid)
                    return 0;
                DYN_STRING_ARRAY dsa1=init_with_size_dyn_string_array(builder->lr1_result->tree->nb_nodes+1);
                unsigned long nb_arg_strings=get_strings_from_arg_list_tkz3_builder(dsa1,
                                                                                    builder->lr1,
                                                                                    builder->lr1_result->tree,
                                                                                    node_arglist);
                if (nb_arg_strings!=1){
                    fprintf(stdout, "Incorrect number of arguments while compiling TKZ3 for function <%s>.\n",function_name);
                    free_dyn_string_array(dsa1);
                    return 0;
                }
                
                // -- Finds list filename
                
                char *list_filename=(char *)get_at_dyn_string_array(dsa1, 0, NULL);
                char *list_path=NULL;
                
                if (builder->ref_directory){
                    list_path=(char *)L_ALLOC(sizeof(char)*(strlen(list_filename)+strlen(builder->ref_directory)+3));
                    sprintf(list_path, "%s/%s",builder->ref_directory,list_filename);
                }
                else
                    list_path=list_filename;
                
                // -- Compiles thefile
                
                FST_O fst=build_from_list_fst_o((unsigned char *)list_path);
                
                if (builder->ref_directory)
                    L_FREE(list_path);
                
                if (!fst){
                    free_dyn_string_array(dsa1);
                    return 0;
                }
                
                FST_O fst2=min_fst_o(NULL, fst);
                
                if (fst2!=fst)
                    free_fst_o(fst);
                
                free_dyn_string_array(dsa1);
                
                // -- Finishes the job
                
                set_fst_for_node_tkz3(node, fst2);
                
                node->is_compiled=1;
                has_changed=1;
                
                mark_completed_recursively_lr1_tree_node(node);

            }



        }

    }
    
    return 1;
}

int compile_line_tkz3_builder(TKZ3_BUILDER builder,char *line){
    
    int res=parse_lr1_buffalo(builder->lr1_result, builder->lr1, builder->lr1_engine, line);
    
    if (!res){
        fprintf(stdout,"UNABLE TO PARSE %s.\n",line);
        return 0;
    }
    
    if (!compile_fso_expression_tkz3_builder(builder, line)){
        return 0;
    }

    // ### TRANSFORMS THE TREE INTO A FST
    SYMB_O symb=builder->lr1->tklex->symb;

    unsigned long start_regex_sid=string_to_label_symb_o_f(symb, "START_REGEX");
    unsigned long end_regex_sid=string_to_label_symb_o_f(symb, "END_REGEX");
    unsigned long equal_sid=string_to_label_symb_o_f(symb, "=");
    unsigned long variable_assignment_sid=string_to_label_symb_o_f(symb, "VARIABLE_ASSIGMENT");
    unsigned long variable_sid=string_to_label_symb_o_f(symb, "VARIABLE");
    unsigned long line_sid=string_to_label_symb_o_f(symb, "LINE");

    unsigned long function_call_sid=string_to_label_symb_o_f(symb, "FUNCTION_CALL");
    
    LR1_TREE tree=builder->lr1_result->tree;
    
    int has_changed=1;
    
    while(has_changed){
        
        has_changed=0;
        
        for(unsigned long i=0;i<tree->nb_nodes;i++){
            
            LR1_TREE_NODE node=get_at_dyn_pointer_array(tree->nodes, i);
            
            if (node->is_compiled)
                continue;
            
            if (node->symbol_id==start_regex_sid ||
                node->symbol_id==end_regex_sid ||
                node->symbol_id==variable_sid ||
                node->symbol_id==equal_sid){
                
                // These are empty nodes
                node->is_compiled=1;
                has_changed=1;
            }
            else if (node->symbol_id==variable_assignment_sid){
                
                if (compile_variable_assignment_tkz3_builder(builder,node)){
                    
                    node->is_compiled=1;
                    has_changed=1;
                    mark_completed_recursively_lr1_tree_node(node);
                }

            }
            else if (node->symbol_id==function_call_sid){
                
                if (compile_function_call_tkz3_builder(builder, node)){
                    
                    node->is_compiled=1;
                    has_changed=1;
                    mark_completed_recursively_lr1_tree_node(node);
                }

            }
            else if (node->symbol_id==line_sid){
                
                if (!check_all_children_compiled(node))
                    continue;
                
                node->is_compiled=1;
                mark_completed_recursively_lr1_tree_node(node);
                has_changed=1;
                
            }

        }

    }
    
    // Cleans up the FSTs
    
    for(unsigned long i=0;i<tree->nb_nodes;i++){
        LR1_TREE_NODE node=get_at_dyn_pointer_array(tree->nodes, i);
        
        if (node->external_data){
            
            FST_O fst=(FST_O)node->external_data;
            if (fst)
                free_fst_o(fst);
            node->external_data=NULL;
            node->is_compiled=0;
        }
    }
    
    return 1;
    
}

#pragma mark - AUX FUNCTIONS

void set_fst_for_node_tkz3(LR1_TREE_NODE node,FST_O fst){
    
    if (node->external_data){
        if (node->external_data==fst)
            return; // already done
        
        FST_O fst0=(FST_O)node->external_data;
        free_fst_o(fst0);
        node->external_data=NULL;
    }
    
    node->external_data=(void *)fst;
    fst->rcount++;
    node->is_compiled=1;
    mark_completed_recursively_lr1_tree_node(node);
    
}

int flush_tag_sets_from_builder_to_tkz3(TKZ3 tkz3,TKZ3_BUILDER builder){
    
    if (!tkz3 || !builder)
        return 0;
    
    if (!builder->nb_tag_sets)
        return 1;
    
    tkz3->nb_tag_sets=builder->nb_tag_sets;

    if (tkz3->tag_set_sizes)
        L_FREE(tkz3->tag_set_sizes);
    tkz3->tag_set_sizes=(unsigned long *)L_ALLOC(sizeof(long)*builder->nb_tag_sets);
    
    if (tkz3->tag_set_poss)
        L_FREE(tkz3->tag_set_poss);
    tkz3->tag_set_poss=(unsigned long *)L_ALLOC(sizeof(long)*builder->nb_tag_sets);
    
    // copies the tag sets into a DYN_STRING_ARRAY
    
    //        DYN_STRING_ARRAY dsa_tagnames=init_with_size_dyn_string_array(builder->nb_tag_sets);
    DYN_LONG_ARRAY d_tag_ids=init_with_size_dyn_long_array(builder->nb_tag_sets);
    
    unsigned long current_pos=0;
    
    for(unsigned long i=0;i<builder->nb_tag_sets;i++){
        TKZ3_BUILDER_TAG_SET tag_set=(TKZ3_BUILDER_TAG_SET)get_at_dyn_pointer_array(builder->tag_sets, i);
        
        tkz3->tag_set_poss[i]=current_pos;
        tkz3->tag_set_sizes[i]=tag_set->nb_tags;
        
        for(unsigned long j=0;j<tag_set->nb_tags;j++){
            unsigned long tag_id=tag_set->tag_ids[j];
            
            set_at_dyn_long_array(d_tag_ids, current_pos++, tag_id);
            
        }
    }
    
    tkz3->size_tag_ids=current_pos;
    
    if (tkz3->size_tag_ids){
        
        if (tkz3->tag_ids)
            L_FREE(tkz3->tag_ids);
        
        tkz3->tag_ids=(unsigned long *)L_ALLOC(sizeof(long)*tkz3->size_tag_ids);
        memcpy(tkz3->tag_ids,d_tag_ids->array,sizeof(long)*tkz3->size_tag_ids);
    }
    free_dyn_long_array(d_tag_ids);
    
    return 1;
}

