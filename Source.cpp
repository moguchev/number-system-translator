#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include <stdbool.h>

const char TRUE[] = "True";
const char FALSE[] = "False";
const char EQUAL = '=';
const char XOR = '+';
const char INVERSION = '!';
const char CONJUNCTION = '&';
const char DISJUNCTION = '|';
const char SEMICOLON = ';';
const char SPACE = ' ';
const char LEFT_BRACKET = '(';
const char RIGHT_BRACKET = ')';
const char ZERO = '0';
const char ONE = '1';

enum { BUFFER_SIZE = 256 };
enum {
    NOT_EXPR = -1,
    WRONG_FORMAT = -2,
    DELIMITER = -3,
    NUMBER = -4,
    NOT_FOUND = -5,
    ERROR = -6
};

const char* RESERVED[] = { "and", "not", "xor", "or", "True", "False" };
const char* SERVICE_CHRS[] = { "&", "!", "+", "|", "1", "0" };

typedef struct node {
    struct node *next;
    char* name;
    bool data;
} node_t;

typedef struct list {
    size_t size;
    node_t *head;
    node_t *tail;
} list_t;

typedef struct stack {
    char c;
    struct stack *next;
} stack_t;

/* �p������� ������� */
void error_msg();
void print_result(int v);
bool is_operator(char c);
bool is_value(char c);
bool is_possible(char c);
bool has_operators(const char *expr);
bool is_right_format(const char *str);

/* ������ �� ������*/
stack_t *push(stack_t *head, char c);
char pop(stack_t **head);
int get_priority(char c);

/* ������ �� ��������*/
/* ������ ��������� �������*/
char *str_replace(char *dst, int num, const char *str,
    const char *orig, const char *rep);
/* �������� ���������� �������*/
size_t remove_ch(char *str, char c);

/* ������ � ������*/
list_t *create_list();
void list_push(list_t *lt, char *name, bool value);
int get_value(list_t *lt, char *name);
void list_pop(list_t *lt);

/* ������� <name>=True|False */
int analyse_and_add(char *expr, list_t *base);
/* ������� ��������� � ���� (!1&0)|1+1 */
int convert(char *expr, list_t *base);
/* ������� � �������� �������� �������*/
char *to_rpn(const char *expr, list_t *base);
/* ���������� */
int calculate(char *expr, list_t *base);



int main()
{
    list_t *list = create_list();
    char buf[BUFFER_SIZE];

    do {
        char *success = fgets(buf, BUFFER_SIZE, stdin);
        if (success == NULL) {
            error_msg();
            break;
        }

        // �������� �� ������������ ������� ������ <name>=True|False
        int status = analyse_and_add(buf, list);
        if (status == WRONG_FORMAT) {
            error_msg();
            break;
        }

        else if (status == NOT_EXPR) {
            int result = calculate(buf, list);
            print_result(result);
            break;
        }
    } while (true);

    while (list->size != 0) {
        list_pop(list);
    }
    free(list);
    return 0;
}


void error_msg() {
    printf("[error]");
}

void print_result(int v)
{
    if (v == 1) {
        printf(TRUE);
    }
    else if (v == 0) {
        printf(FALSE);
    }
    else {

        error_msg();
    }
}

bool is_operator(char c)
{
    return strchr("!&|+", c) != NULL;
}

bool is_value(char c)
{
    return c == ONE || c == ZERO;
}

bool is_possible(char c)
{
    return strchr("() \n\t", c) == NULL && !is_operator(c) && !is_value(c);
}

bool has_operators(const char *expr)
{
    bool no_operators = strchr(expr, XOR) == NULL &&
        strchr(expr, DISJUNCTION) == NULL &&
        strchr(expr, CONJUNCTION) == NULL &&
        strchr(expr, INVERSION) == NULL;

    return !no_operators;
}

bool is_right_format(const char *str)
{
    bool only_small = false;
    // �������� �� ��������� ��������� �����
    for (size_t i = 0; i < strlen(str); ++i) {
        if (str[i] >= 'a' && str[i] <= 'z') {
            only_small = true;
        }
        else {
            only_small = false;
            break;
        }
    }

    // �������� �� ����������� ����� and or not xor;
    if (only_small) {
        for (size_t k = 0; RESERVED[k]; ++k) {
            if (strcmp(str, RESERVED[k]) == 0)
                return false;
        }
    }

    return only_small;
}

/* ������� push ���������� �� ���� (�� ��p���� ����p��� ��������� HEAD)
   ������ a . ����p����� ��������� �� ����� ��p���� ����� */
stack_t *push(stack_t *head, char c)
{
    stack_t *ptr = (stack_t*)malloc(sizeof(stack_t));
    if (ptr == NULL) {
        /* ���� � ��� - ����� */
        error_msg();
        return NULL;
    }
    /* ������������� ��������� ��p���� */
    ptr->c = c;
    /* � ����������� � � ����� */
    ptr->next = head;
    /* PTR -����� ��p���� ����� */
    return ptr;
}

/* ������� pop ������� ������ � ��p���� �����.
   ����p����� ��������� ������.
   �������� ��������� �� ��p���� ����� */
char pop(stack_t **head)
{
    stack_t *ptr = NULL;
    char el;
    /* ���� ���� ����,  ����p������� '\0' */
    if (*head == NULL)
        return '\0';
    /* � PTR - ��p�� ��p���� ����� */
    ptr = *head;
    el = ptr->c;
    /* �������� ��p�� ��p���� ����� */
    *head = ptr->next;
    /* ������������ ������ */
    free(ptr);
    /* ����p�� ������� � ��p���� ����� */
    return el;
}

/* �������  ����p����� �p��p����  ���p���� */
int get_priority(char c)
{
    switch (c)
    {
    case '!':
        return 5;
    case '&':
        return 4;
    case '|':
        return 3;
    case '+':
        return 2;
    case '(':
        return 1;
    default:
        return 0;
    }
}

char *str_replace(char *dst, int size, const char *str,
    const char *orig, const char *rep) {
    if (!str || !orig || !rep) {
        return NULL;
    }
    const char* ptr;
    size_t len1 = strlen(orig);
    size_t len2 = strlen(rep);
    char*  tmp = dst;

    size -= 1;
    while ((ptr = strstr(str, orig)) != NULL) {
        size -= (ptr - str) + len2;
        if (size < 1)
            break;

        strncpy(dst, str, (size_t)(ptr - str));
        dst += ptr - str;
        strncpy(dst, rep, len2);
        dst += len2;
        str = ptr + len1;
    }

    for (; (*dst = *str) && (size > 0); --size) {
        ++dst;
        ++str;
    }
    return tmp;
}

size_t remove_ch(char *s, char ch)
{
    size_t i = 0, j = 0;
    for (i = 0; s[i]; ++i)
        if (s[i] != ch)
            s[j++] = s[i];

    s[j] = '\0';
    return j;
}

list_t* create_list()
{
    list_t *lt = (list_t*)malloc(sizeof(list_t));
    if (!lt)
        return NULL;

    lt->size = 0;
    lt->head = NULL;
    lt->tail = lt->head;

    return lt;
}

void list_push(list_t *lt, char *name, bool value)
{
    if (lt == NULL)
        return;

    node_t* node = (node_t*)malloc(sizeof(node_t));
    if (!node)
        return;

    node->name = name;
    node->data = value;
    node->next = lt->head;

    lt->head = node;
    lt->size += 1;
}

int get_value(list_t *lt, char* name)
{
    node_t *curr = lt->head;
    while (curr != NULL) {
        if (strcmp(name, curr->name) == 0)
            return curr->data;
        curr = curr->next;
    }
    return NOT_FOUND;
}

void list_pop(list_t *lt)
{
    if ( lt == NULL || lt->size == 0) {
        return;
    }

    node_t *node = lt->head;
    free(lt->head->name);
    lt->size -= 1;
    lt->head = node->next;
    free(node);

    if (lt->size == 0) {
        lt->head = NULL;
        lt->tail = NULL;
    }
}

// ������ � ���� ���� <name>,  <True|False>
int analyse_and_add(char *expr, list_t *base)
{
    if (!expr)
        return ERROR;

    // ���� �� <name>=True|False
    char* pos = strchr(expr, EQUAL);
    char* end = strchr(expr, SEMICOLON);
    if (pos == NULL || end == NULL)
        return NOT_EXPR;

    remove_ch(expr, SPACE);
    pos = strchr(expr, EQUAL);
    end = strchr(expr, SEMICOLON);

    /* ���������� ���*/
    size_t len_n = pos - expr;
    char* name = (char*)calloc(len_n + 1, sizeof(char));
    if (!name) {
        return ERROR;
    }
    memmove(name, expr, sizeof(char) * len_n);

    if (!is_right_format(name)) {
        free(name);
        return WRONG_FORMAT;
    }

    /* �������� ��������*/
    size_t len_v = end - pos;
    char* value = (char*) calloc(len_v, sizeof(char));
    if (!value) {
        free(name);
        return ERROR;
    }
    memmove(value, pos + 1, sizeof(char) * (len_v-1));

    /* ���������� True|False */
    bool v;
    if (strcmp(value, TRUE) == 0)
        v = true;
    else if (strcmp(value, FALSE) == 0)
        v = false;
    else
    {
        free(name);
        free(value);
        return WRONG_FORMAT;
    }

    /* ���������� � ���� ����-��������*/
    list_push(base, name, v);
    free(value);
    return v;
}

int convert(char *expr, list_t *base)
{
    if (!expr || base == NULL)
        return ERROR;

    char temp[BUFFER_SIZE];

    /* ������ �������� ���� �� �������������� ������� */
    size_t i = 0;
    for (i = 0; RESERVED[i] && SERVICE_CHRS[i]; ++i) {
        if(str_replace(temp, sizeof(temp) - 1, expr, RESERVED[i], SERVICE_CHRS[i]) == NULL)
            return ERROR;
        else
            strcpy(expr, temp);
    }
    
    /* ������ ���������� �� �� �������� */
    for (i = 0; expr[i]; ++i) {
        if (is_possible(expr[i])) {
            size_t j = i;
            for (j = i; is_possible(expr[j]) && expr[j]; ++j);

            char* varible = (char*)calloc(j - i + 1, sizeof(char));
            if (!varible) {
                return ERROR;
            }
            memmove(varible, expr + i, j - i);

            if (strlen(varible) == 0)
                break;

            int status = get_value(base, varible);
            if (status == NOT_FOUND)  {
                free(varible);
                return NOT_FOUND;
            }

            if (status == 0)
                if (str_replace(temp, sizeof(temp) - 1, expr, varible, "0") == NULL)
                    return ERROR;
            if (status == 1)
                if (str_replace(temp, sizeof(temp) - 1, expr, varible, "1") == NULL)
                    return ERROR;

            strcpy(expr, temp);
            free(varible);
        }
    }

    /* �������� ��������*/
    remove_ch(expr, SPACE);

    /* �����*/
    return 0;
}

char* to_rpn(const char *expr, list_t *base)
{
    stack_t *operators = NULL;
    char* expr_in_rpn_format = (char*)malloc(BUFFER_SIZE * sizeof(char));
    if (expr_in_rpn_format == NULL)
        return NULL;
    int k, point;

    k = point = 0;
    /* �����p��� , ���� �� ������ �� ����� */
    while (expr[k])
    {
        /* ���� ���p����� ������ - ')' */
        if (expr[k] == RIGHT_BRACKET)
            /* �� ����������� �� ����� � �������� ��p��� */
        {
            /* ��� ����� ���p���� �� ��������� */
            while ((operators->c) != LEFT_BRACKET)
                /* ���p������� ������ */
                expr_in_rpn_format[point++] = pop(&operators);
            /* ������� �� ����� ���� ���p������� ������ */
            pop(&operators);
        }
        /* ���� ���p����� ������ - �������� , �� */
        if (is_value(expr[k]))
            /* ��p��������� � � �������� ��p��� */
            expr_in_rpn_format[point++] = expr[k];
        /* ���� ���p����� ������ - '(' , �� */
        if (expr[k] == LEFT_BRACKET)
            /* ����������� � � ���� */
            operators = push(operators, LEFT_BRACKET);
        if (is_operator(expr[k]))
            /* ���� ��������� ������ - ���� ���p���� , ��: */
        {
            /* ���� ���� ���� */
            if (operators == NULL)
                /* ���������� � ���� ���p���� */
                operators = push(operators, expr[k]);
            /* ���� �� ���� */
            else
                /* ���� �p��p���� ����������� ���p���� ������
                                �p��p����� ���p���� �� ��p���� ����� */
                if (get_priority(operators->c) < get_priority(expr[k]))
                    /* ����������� ����������� ���p���� �� ���� */
                    operators = push(operators, expr[k]);
            /* ���� �p��p���� ������ */
                else
                {
                    while ((operators != NULL) && (get_priority(operators->c) >= get_priority(expr[k])))
                        /* ��p��������� � �������� ��p��� ��� ���p����
                                            � ������� ��� p����� �p��p������ */
                        expr_in_rpn_format[point++] = pop(&operators);
                    /* ���������� � ���� �����������  ���p���� */
                    operators = push(operators, expr[k]);
                }
        }
        /* ��p���� � ���������� ������� ������� ��p��� */
        k++;
    }
    /* ����� p������p���� ����� ��p������ */
    while (operators != NULL)
        /* ��p��������� ��� ���p���� �� */
        expr_in_rpn_format[point++] = pop(&operators);
    /* ����� � �������� ��p��� */
    expr_in_rpn_format[point] = '\0';

    return expr_in_rpn_format;
}

int calculate(char *expr, list_t *base)
{
    if (convert(expr, base) != 0)
        return ERROR;

    char* outstring = to_rpn(expr, base);
    if (outstring == NULL)
        return ERROR;

    // ���� ��� ���������� � ������ 1 ����������
    if (strlen(outstring) > 1 && !has_operators(outstring))  {
        free(outstring);
        return ERROR;
    }
    
    /* �������� ����� ��� �������� ���������� */
    int *stack = (int*)malloc(strlen(expr)*sizeof(int));
    if (stack == NULL) {
        free(outstring);
        return ERROR;
    }
    // sp = ������ ������, ���� ����� push-����� ��������� �����
    int sp = 0;      // (sp-1) �������� �����
    for (size_t k = 0; outstring[k]; ++k) {
        char c = outstring[k];
        switch (c) {
        case '\n':
            break;
        case '+':
            stack[sp - 2] ^= stack[sp - 1];  sp--;
            break;
        case '|':
            stack[sp - 2] |= stack[sp - 1];  sp--;
            break;
        case '&':
            stack[sp - 2] = stack[sp - 1] & stack[sp - 2];  sp--;
            break;
        case '!':
            stack[sp - 1] = !stack[sp - 1];
            break;
        case '0': {
            stack[sp++] = 0;
            break;
        }
        case '1': {
            stack[sp++] = 1;
            break;
        }
        default:
            free(stack);
            free(outstring);
            return WRONG_FORMAT;
        }
    }

    int result = stack[sp - 1];
    free(outstring);
    free(stack);
    if (result == 0 || result == 1)
        return result;

    return ERROR;
}