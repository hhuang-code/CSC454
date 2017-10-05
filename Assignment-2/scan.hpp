#ifndef SCAN_HPP
#define SCAN_HPP

#include <queue>

using namespace std;

// Terminal
typedef enum {t_read, t_write, t_id, t_literal, t_gets,
    t_add, t_sub, t_mul, t_div, 
    t_lparen, t_rparen,
    t_eq, t_neq, t_lte, t_gt, t_lt, t_gte,
    t_if, t_fi, t_do, t_od, t_check,
    t_eoa, t_eof, t_eps
} token;

// Non-terminal
typedef enum {nt_program, nt_stmt_list, nt_stmt, nt_relation, 
    nt_expr_tail, nt_expr, nt_term_tail, nt_term, 
    nt_factor_tail, nt_factor, nt_ro_op, nt_ao_op, nt_mo_op
} symbol;

extern char token_image[100];

extern token scan();

// Store parsing error message.
extern queue<string> error_msg;

// Record line number
extern int linecnt;

#endif
