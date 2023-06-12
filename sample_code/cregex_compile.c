// cregex_compile.c

#include"lang.h"


static int trace=0;
static int deep_trace=0;

FST_O get_recursive_fst_lr1_tree_node(LR1_TREE_NODE node);
char get_char_from_char_node(LR1_TREE_NODE node);
FST_O special_range_fst(LR1_TREE_NODE node);

void set_trace_cregex_compile(int the_trace){
    trace=the_trace;
}

// TO BE PUT INTO L5_RESULT and L5_ENGINE

// Takes a string of the shape
// "/[A-Z]+\w*[^cd]+/"
//    or
// "[A-Z]+\w*[^cd]+"
//
// and compiles it into an FSTO
// NOTE: the input string can either has '/' surrounding it or just the content of it
// The function figures out on its own which case it is
//
// base_dir : used in case of meta regex to give a path to files
FST_O compile_cregex(char *original_input_string,char *base_dir){
    
    if (!original_input_string || *original_input_string=='\0')
        return NULL;
    
    if (*original_input_string=='|' && original_input_string[1]=='{'){
        return compile_meta_regex(original_input_string, base_dir);
    }
    
    char *input_string=NULL;
    
    // If the string has the shape "/.../" , removes the first and last '/'
    if (strlen(original_input_string)>2 &&
        *original_input_string=='/' &&
        original_input_string[strlen(original_input_string)-1]=='/'){
        
        input_string=(char *)L_ALLOC(sizeof(char)*(strlen(original_input_string)+1));
        strcpy(input_string,&(original_input_string[1])); // removes the first character: the '/'
        input_string[strlen(input_string)-1]='\0'; // removes the trailing '/'
    }
    else
        input_string=original_input_string;

    //LR1_BUFFALO lr1=build_lr1_buffalo_cregex();
    LR1_BUFFALO lr1=get_shared_lr1_buffalo_regex();
    
    LR1_RESULT result=init_lr1_result();
    LR1_ENGINE lr1_engine=init_lr1_engine(lr1);
    
    // ### PARSING


    int res=parse_lr1_buffalo(result, lr1, lr1_engine, input_string);
    
    if (!res){
        fprintf(stdout,"UNABLE TO PARSE %s.\n",input_string);
        return NULL;
    }
    
    SYMB_O symb=lr1->tklex->symb;
    
    unsigned long item_sid=string_to_label_symb_o_f(symb, "ITEM");
    unsigned long item_list_sid=string_to_label_symb_o_f(symb, "ITEM_LIST");
    unsigned long item_plus_sid=string_to_label_symb_o_f(symb, "ITEM_PLUS");
    unsigned long item_star_sid=string_to_label_symb_o_f(symb, "ITEM_STAR");
    unsigned long item_question_sid=string_to_label_symb_o_f(symb, "ITEM_QUESTION");
    unsigned long char_range_sid=string_to_label_symb_o_f(symb, "CHAR_RANGE");
    unsigned long set_content_item_sid=string_to_label_symb_o_f(symb, "SET_CONTENT_ITEM");
    unsigned long set_content_sid=string_to_label_symb_o_f(symb, "SET_CONTENT");
    unsigned long open_sq_sid=string_to_label_symb_o_f(symb, "OPEN_SQ");
    unsigned long close_sq_sid=string_to_label_symb_o_f(symb, "CLOSE_SQ");
    unsigned long open_not_sid=string_to_label_symb_o_f(symb, "OPEN_NOT");
    unsigned long open_par_sid=string_to_label_symb_o_f(symb, "OPEN_PAR");
    unsigned long close_par_sid=string_to_label_symb_o_f(symb, "CLOSE_PAR");
    unsigned long or_set_sid=string_to_label_symb_o_f(symb, "OR_SET");
    unsigned long or_set_content_sid=string_to_label_symb_o_f(symb, "OR_SET_CONTENT");
    unsigned long or_sid=string_to_label_symb_o_f(symb, "OR");
    unsigned long special_range_sid=string_to_label_symb_o_f(symb, "SPECIAL_RANGE");


    unsigned long set_sid=string_to_label_symb_o_f(symb, "SET");
    unsigned long not_range_set_sid=string_to_label_symb_o_f(symb, "NOT_RANGE_SET");

    unsigned long char_sid=string_to_label_symb_o_f(symb, "char");
    
    // ### TRANSFORMS THE TREE INTO A FST
    
    LR1_TREE tree=result->tree;

    int has_changed=1;
    
    while(has_changed){
        
        has_changed=0;
        
        for(unsigned long i=0;i<tree->nb_nodes;i++){
            
            LR1_TREE_NODE node=get_at_dyn_pointer_array(tree->nodes, i);
            
            if (node->is_compiled)
                continue;

            if (node->symbol_id==open_sq_sid ||
                node->symbol_id==close_sq_sid ||
                node->symbol_id==or_sid ||
                node->symbol_id==open_not_sid ||
                node->symbol_id==open_par_sid ||
                node->symbol_id==close_par_sid){
                
                // These are empty nodes
                node->is_compiled=1;
            }
            else if (node->symbol_id==item_sid){
                
                if (node->nb_children==1){ // test ' char'
                    
                    LR1_TREE_NODE node2=get_at_dyn_pointer_array(node->children,0);
                    
                    if (node2->symbol_id==char_sid){
                        
                        FST_O fst=fst_l_444_to_fst_o(init_fst_l_444());
                        node->external_data=(void *)fst;
                        
                        add_state_FST_O(fst);
                        set_state_type_FST_O(fst, 1, 5);
                        
                        char c=get_char_from_char_node(node2);
                        add_trans_FST_O(NULL, fst, 0, 1, c, c, 0.0);
                        
                        mark_completed_recursively_lr1_tree_node(node);
                        has_changed=1;
                        
                        if (deep_trace)
                            display_std_fst_o(fst);

                        
                    }
                    else if (node2->is_compiled){
                        node->is_transparent=1;
                        node->is_compiled=1;
                        has_changed=1;
                        continue;
                    }
                    
                }
            }
            else if (node->symbol_id==item_list_sid){
                
                if (!check_all_children_compiled(node))
                    continue;

                if (node->nb_children==1){
                    node->is_transparent=1;
                    node->is_compiled=1;
                    has_changed=1;
                    continue;

                }

                FST_O *fsts=(FST_O *)L_ALLOC(sizeof(FST_O)*node->nb_children);
                
                for(unsigned long j=0;j<node->nb_children;j++){
                    LR1_TREE_NODE node2=get_at_dyn_pointer_array(node->children,j);
                    fsts[j]=get_recursive_fst_lr1_tree_node(node2);
                }
                
                FST_O fst=concat_lists_fst_o(node->nb_children, fsts);
                
                L_FREE(fsts);
                
                node->external_data=(FST_O)fst;
                node->is_compiled=1;
                
                
            }
            else if (node->symbol_id==item_plus_sid){
                
                if (node->nb_children==2){ // test ' char'
                    
                    LR1_TREE_NODE node2=get_at_dyn_pointer_array(node->children,0);
                    
                    if (node2->symbol_id==char_sid){
                        
                        FST_O fst=fst_l_444_to_fst_o(init_fst_l_444());
                        node->external_data=(void *)fst;
                        
                        add_state_FST_O(fst);
                        set_state_type_FST_O(fst, 1, 5);
                        
                        char c=get_char_from_char_node(node2);

                        add_trans_FST_O(NULL, fst, 0, 1, c, c, 0.0); // the char transition
                        add_trans_FST_O(NULL, fst, 1, 0, 0, 0, 0.0); // the epsilon transition

                        mark_completed_recursively_lr1_tree_node(node);
                        has_changed=1;
                        
                        if (deep_trace)
                            display_std_fst_o(fst);
                    }
                    else if (node2->symbol_id==set_sid && node2->is_compiled){
                        
                        FST_O fst1=get_recursive_fst_lr1_tree_node(node2);
                        // (FST_O)node2->external_data;
                        FST_O fst=plus_fst_o(fst1);
                        
                        node->external_data=fst;
                        
                        node->is_compiled=1;
                        mark_completed_recursively_lr1_tree_node(node);
                        has_changed=1;
                        continue;

                    }

                }
            }
            else if (node->symbol_id==item_star_sid){
                
                if (node->nb_children==2){ // test ' char'
                    
                    LR1_TREE_NODE node2=get_at_dyn_pointer_array(node->children,0);
                    
                    if (node2->symbol_id==char_sid){
                        
                        FST_O fst=fst_l_444_to_fst_o(init_fst_l_444());
                        node->external_data=(void *)fst;
                        
                        add_state_FST_O(fst);
                        set_state_type_FST_O(fst, 1, 5);
                        set_state_type_FST_O(fst, 0, 5);

                        char c=get_char_from_char_node(node2);
                        
                        add_trans_FST_O(NULL, fst, 0, 1, c, c, 0.0); // the char transition
                        add_trans_FST_O(NULL, fst, 1, 0, 0, 0, 0.0); // the epsilon transition
                        
                        mark_completed_recursively_lr1_tree_node(node);
                        has_changed=1;
                        
                        if (deep_trace)
                            display_std_fst_o(fst);
                    }
                    else if (node2->symbol_id==set_sid && node2->is_compiled){
                        
                        FST_O fst1=get_recursive_fst_lr1_tree_node(node2);
                        FST_O fst=star_fst_o(fst1);
                        
                        node->external_data=fst;
                        
                        node->is_compiled=1;
                        mark_completed_recursively_lr1_tree_node(node);
                        has_changed=1;
                        continue;
                    }
                    
                }
            }
            else if (node->symbol_id==item_question_sid){
                
                if (node->nb_children==2){ // test ' char'
                    
                    LR1_TREE_NODE node2=get_at_dyn_pointer_array(node->children,0);
                    
                    if (node2->symbol_id==char_sid){
                        
                        FST_O fst=fst_l_444_to_fst_o(init_fst_l_444());
                        node->external_data=(void *)fst;
                        
                        add_state_FST_O(fst);
                        set_state_type_FST_O(fst, 1, 5);
                        set_state_type_FST_O(fst, 0, 5);
                        
                        char c=get_char_from_char_node(node2);
                        
                        add_trans_FST_O(NULL, fst, 0, 1, c, c, 0.0); // the char transition
                        
                        mark_completed_recursively_lr1_tree_node(node);
                        has_changed=1;
                        
                        if (deep_trace)
                            display_std_fst_o(fst);
                    }
                    else if (node2->symbol_id==set_sid && node2->is_compiled){
                        
                        FST_O fst1=get_recursive_fst_lr1_tree_node(node2);
                        FST_O fst=question_fst_o(fst1);
                        
                        node->external_data=fst;
                        
                        node->is_compiled=1;
                        mark_completed_recursively_lr1_tree_node(node);
                        has_changed=1;
                        continue;
                    }
                    
                }
            }
            else if (node->symbol_id==char_range_sid){
                
                if (node->nb_children==3){ // char DASH char
                    LR1_TREE_NODE node2=get_at_dyn_pointer_array(node->children,0);
                    LR1_TREE_NODE node3=get_at_dyn_pointer_array(node->children,2);
                    
                    if (node2 && node3 && node2->symbol_id==char_sid && node2->symbol_id==char_sid){
                        
                        char c1=get_char_from_char_node(node2);
                        char c2=get_char_from_char_node(node3);
                        
                        if (c2<c1){
                            fprintf(stdout, "ERROR _cregex (3): range [%c-%c] is empty\n",c1,c2);
                            return NULL;
                        }
                        
                        FST_O fst=fst_l_444_to_fst_o(init_fst_l_444());
                        node->external_data=(void *)fst;
                        
                        add_state_FST_O(fst);
                        set_state_type_FST_O(fst, 1, 5);

                        for(char c=c1;c<=c2;c++)
                            add_trans_FST_O(NULL, fst, 0, 1, c, c, 0.0);
                        
                        mark_completed_recursively_lr1_tree_node(node);
                        has_changed=1;
                        
                        if (deep_trace)
                            display_std_fst_o(fst);
                        
                    }
                    else{
                        fprintf(stdout,"ERROR _cregex (4)\n");
                        return NULL;
                    }

                }
                else if (node->nb_children==1){
                    LR1_TREE_NODE node2=get_at_dyn_pointer_array(node->children,0);
                    
                    if (!node2->is_compiled)
                        continue;
                    
                    if (node2->symbol_id==special_range_sid){
                        node->is_transparent=1;
                        node->is_compiled=1;
                        has_changed=1;
                        continue;
                    }

                }
                else{
                    fprintf(stdout,"ERROR _cregex (5)\n");
                    return NULL;
                }
                
            }
            else if (node->symbol_id==set_content_item_sid){
                
                LR1_TREE_NODE node2=get_at_dyn_pointer_array(node->children,0);
                
                if (node2->symbol_id==char_sid){
                    
                    FST_O fst=fst_l_444_to_fst_o(init_fst_l_444());
                    node->external_data=(void *)fst;
                    
                    add_state_FST_O(fst);
                    set_state_type_FST_O(fst, 1, 5);
                    
                    char c=get_char_from_char_node(node2);

                    add_trans_FST_O(NULL, fst, 0, 1, c, c, 0.0);
                    
                    mark_completed_recursively_lr1_tree_node(node);
                    has_changed=1;
                    
                    if (deep_trace)
                        display_std_fst_o(fst);
                    
                    continue;
                }

                
                if (!check_all_children_compiled(node))
                    continue;
                
                if (node->nb_children==1){
                    node->is_transparent=1;
                    node->is_compiled=1;
                    has_changed=1;
                    continue;
                }
                else{
                    fprintf(stdout, "ERROR _cregex (6)\n");
                    return NULL;
                }

            }
            else if (node->symbol_id==set_content_sid){
                
                if (!check_all_children_compiled(node))
                    continue;
                
                if (node->nb_children==1){
                    node->is_transparent=1;
                    node->is_compiled=1;
                    has_changed=1;
                    continue;
                }
                
                // OR (disjunction) of all the FSTs
                
                FST_O *fsts=(FST_O *)L_ALLOC(sizeof(FST_O)*node->nb_children);
                
                for(unsigned long j=0;j<node->nb_children;j++){
                    LR1_TREE_NODE node2=get_at_dyn_pointer_array(node->children,j);
                    fsts[j]=get_recursive_fst_lr1_tree_node(node2);
                }
                
                //FST_O fst=or_lists_fst_o(node->nb_children, fsts);
                // The Opimtized form for chars FST:
                FST_O fst=or_lists_chars_fst_o(node->nb_children, fsts);

                L_FREE(fsts);
                
                node->external_data=(FST_O)fst;
                node->is_compiled=1;
                mark_completed_recursively_lr1_tree_node(node);

            }
            else if (node->symbol_id==set_sid){
                
                if (!check_all_children_compiled(node))
                    continue;
                
                if (node->nb_children==3){  // OPEN_SQ SET_CONTENT CLOSE_SQ
                    
                    LR1_TREE_NODE node2=get_at_dyn_pointer_array(node->children,1);
                    FST_O fst1=get_recursive_fst_lr1_tree_node(node2);

                    FST_O fst=cpy_fst_o(fst1);
                    
                    node->external_data=(void *)fst;
                    
                    node->is_compiled=1;
                    has_changed=1;
                    continue;

                }
                else if (node->nb_children==1){
                    node->is_transparent=1;
                    node->is_compiled=1;
                    has_changed=1;
                    continue;

                }
                else{
                    fprintf(stdout,"ERROR _cregex(7)\n");
                    return NULL;
                }
            }
            else if (node->symbol_id==not_range_set_sid){
                
                // ER TODO
                
                if (!check_all_children_compiled(node))
                    continue;
                
                if (node->nb_children==3){  // OPEN_NOT SET_CONTENT CLOSE_SQ
                    
                    LR1_TREE_NODE node2=get_at_dyn_pointer_array(node->children,1);
                    FST_O fst1=get_recursive_fst_lr1_tree_node(node2);
                    
                    FST_O fst=not_char_fst(fst1);
                    
                    node->external_data=(void *)fst;
                    
                    node->is_compiled=1;
                    has_changed=1;
                    continue;
                    
                }
                else{
                    fprintf(stdout,"ERROR _cregex(7)\n");
                    return NULL;
                }
            }

            else if (node->symbol_id==or_set_sid){
                
                if (!check_all_children_compiled(node))
                    continue;
                
                if (node->nb_children==5){  // OPEN_PAR ITEM_LIST OR ITEM_LIST CLOSE_PAR
                    
                    // OBSOLETE

                    FST_O *fsts=(FST_O *)L_ALLOC(sizeof(FST_O)*node->nb_children);
                    
                    LR1_TREE_NODE node2=get_at_dyn_pointer_array(node->children,1);
                    fsts[0]=get_recursive_fst_lr1_tree_node(node2);
                    node2=get_at_dyn_pointer_array(node->children,3);
                    fsts[1]=get_recursive_fst_lr1_tree_node(node2);
                    
                    FST_O fst=or_lists_fst_o(2, fsts);
                    node->external_data=(void *)fst;
                    
                    L_FREE(fsts);
                    
                    node->is_compiled=1;
                    mark_completed_recursively_lr1_tree_node(node);

                    continue;
                }
                else if (node->nb_children==3){ // OPEN_PAR OR_SET_CONTENT CLOSE_PAR
                    LR1_TREE_NODE node2=get_at_dyn_pointer_array(node->children,1);
                    FST_O fst1=get_recursive_fst_lr1_tree_node(node2);
                    
                    FST_O fst=cpy_fst_o(fst1);
                    
                    node->external_data=(void *)fst;
                    node->is_compiled=1;

                }
            }
            else if (node->symbol_id==or_set_content_sid){
                
                if (!check_all_children_compiled(node))
                    continue;
                
                if (node->nb_children==3){
                    // either
                    // ITEM_LIST OR ITEM_LIST
                    // or
                    // ITEM_LIST OR OR_SET_CONTENT
                    // but same behavior in both cases
                    
                    
                    FST_O *fsts=(FST_O *)L_ALLOC(sizeof(FST_O)*node->nb_children);
                    
                    LR1_TREE_NODE node2=get_at_dyn_pointer_array(node->children,0);
                    fsts[0]=get_recursive_fst_lr1_tree_node(node2);
                    node2=get_at_dyn_pointer_array(node->children,2);
                    fsts[1]=get_recursive_fst_lr1_tree_node(node2);
                    
                    FST_O fst=or_lists_fst_o(2, fsts);
                    node->external_data=(void *)fst;
                    
                    L_FREE(fsts);
                    
                    node->is_compiled=1;
                    mark_completed_recursively_lr1_tree_node(node);
                    
                    continue;
                }
            }
            else if (node->symbol_id==special_range_sid){
                
                FST_O fst=special_range_fst(node);
                
                node->external_data=(void *)fst;
                
                node->is_compiled=1;
                mark_completed_recursively_lr1_tree_node(node);

            }

        }

    }
    
    // ## Gets the top level FST
    
    LR1_TREE_NODE root_node=get_at_dyn_pointer_array(tree->nodes,tree->root_node_id);
    
    if (!root_node->is_compiled){
        fprintf(stdout,"Compilation incomplete (1).\n");
        return 0;
    }
    
    FST_O root_fst=get_recursive_fst_lr1_tree_node(root_node);

    if (trace){
        fprintf(stdout,"CREGEX COMPILATION RESULT:\n");
        display_std_fst_o(root_fst);
    }
    
    // ## Optimizes (epsilon-removal, determinization, minimization)
    
    FST_O final_fst=optimize_fst_o(root_fst);
    
    if (trace){
        fprintf(stdout,"CREGEX COMPILATION RESULT AFTER OPTIMIZATION:\n");
        display_std_fst_o(final_fst);
    }

    
    // ## Frees the TMP transducers
    
    for(unsigned long i=0;i<tree->nb_nodes;i++){
        LR1_TREE_NODE node=get_at_dyn_pointer_array(tree->nodes, i);
        
        if (!node->external_data)
            continue;
        
        FST_O fst=(FST_O)node->external_data;
        
        free_fst_o(fst);
        
        node->external_data=NULL;
    }
    
    // ## Frees the working structures
    
    free_lr1_engine(lr1_engine);
    free_lr1_result(result);
    
    if (input_string!=original_input_string)
        L_FREE(input_string);
    
    //free_lr1_buffalo(lr1);
    
    return final_fst;
}

FST_O get_recursive_fst_lr1_tree_node(LR1_TREE_NODE node){
    if (!node->is_compiled)
        return NULL;
    
    if (!node->is_transparent){
        return (FST_O)node->external_data;
    }
    
    if (!node->nb_children)
        return NULL;
    
    LR1_TREE_NODE child=get_at_dyn_pointer_array(node->children, 0);
    
    return get_recursive_fst_lr1_tree_node(child);
    
}

char get_char_from_char_node(LR1_TREE_NODE node){
    char c=node->original_token->char_array->array[0];
    
    if (c=='\\'){
        c=node->original_token->char_array->array[1];
        
        switch (c) {
            case 'n':
                return (char)10;
                break;
            case 'r':
                return (char)13;
                break;
            case 't':
                return (char)9;
                break;
            case 'f':
                return (char)12;
                break;
            default:
                break;
        }
    }
    

    return c;
}

// handles \d \w \s and .
FST_O special_range_fst(LR1_TREE_NODE node){
    
    FST_O fst=fst_l_444_to_fst_o(init_fst_l_444());
    add_state_FST_O(fst);
    
    set_state_type_FST_O(fst, 1, 5);
    
    char c=node->original_token->char_array->array[0];
    
    if (c=='.'){
        
        for(unsigned char c=1;c<253;c++){
            if (c!=10 && c!=13){
                add_trans_FST_O(NULL, fst, 0, 1, c, c, 0.0);
            }
        }
        return fst;
    }
    
    if (c=='\\'){
        c=node->original_token->char_array->array[1];
        
        switch (c) {
            case 'w':
                {
                    for(unsigned char c='a';c<='z';c++){
                        add_trans_FST_O(NULL, fst, 0, 1, c, c, 0.0);
                    }
                    for(unsigned char c='A';c<='Z';c++){
                        add_trans_FST_O(NULL, fst, 0, 1, c, c, 0.0);
                    }

                    for(unsigned char c='0';c<='9';c++){
                        add_trans_FST_O(NULL, fst, 0, 1, c, c, 0.0);
                    }
                    add_trans_FST_O(NULL, fst, 0, 1, '_','_', 0.0);

                }
                break;
            case 's':
            {
                add_trans_FST_O(NULL, fst, 0, 1, ' ',' ', 0.0);
                add_trans_FST_O(NULL, fst, 0, 1, 9, 9, 0.0);
                add_trans_FST_O(NULL, fst, 0, 1, 10, 10, 0.0);
                add_trans_FST_O(NULL, fst, 0, 1, 12, 12, 0.0);
                add_trans_FST_O(NULL, fst, 0, 1, 13, 13, 0.0);


            }
                break;
            case 'd':
            {
                for(unsigned char c='0';c<='9';c++){
                    add_trans_FST_O(NULL, fst, 0, 1, c, c, 0.0);
                }
            }
                break;

            default:
                break;
        }
 
    }
    else{
        fprintf(stdout,"ERROR: CREGEX special range error\b");
        free_fst_o(fst);
        return NULL;
    }
    

    return fst;
}


