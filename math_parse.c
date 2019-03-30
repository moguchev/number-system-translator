// Парсер математических выражений: рекурсивный однопроходный парсер
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <malloc.h>

int* getToken(char*); //Получает лексему из строки
void pars(char*); //Точка входа анализатора
int fSum(double*); //Обрабатывает сложение и вычитание
int fMulti(double*); //Обрабатывает умножение и деление
int fExp(double*); //Возведение в степень
int fUnary(double*); //Обработка унарных операторов
int fBrack(double*); //Обрабатывает выражение в скобках
int fAtom(double*); //Получает значение числа

char *expr; //Указатель на обрабатываемую строку
char token[80]; //Лексема
enum { Empty, Operator, Variable, Number } type; //Тип лексемы
enum { No, Syntax, Zero } error; //Значение ошибки

void pars(char *line)
{
    int *pointer;
    double result;
    error = No;
    expr = line;
    pointer = getToken(expr);
    fSum(&result);
    *pointer = 0;

    switch (error)
    {
    case No:
        sprintf(expr, "%f", result);
        break;
    case Syntax:
        strcpy(expr, "Syntax error!");
        break;
    case Zero:
        strcpy(expr, "Divide by zero!");
        break;
    }
}

int* getToken(char *expr)
{
    static int i = 0;
    type = Empty;

    if (expr[i] == '\0') //Если конец выражения
    {
        i = 0;
        return 0;
    }
    while (isspace(expr[i])) i++; //Пропустить разделительные символы

    if (strchr("+-*/%^=()", expr[i]))
    {
        *token = expr[i];
        *(token + 1) = '\0';
        type = Operator;
    }
    else if (isalpha(expr[i]))
    {
        *token = expr[i];
        *(token + 1) = '\0';
        type = Variable;
    }
    else if (isdigit(expr[i]))
    {
        int j = 0;
        token[j] = expr[i];
        while (isdigit(expr[i + 1]) || expr[i + 1] == '.')
            token[++j] = expr[++i];
        token[j + 1] = '\0';
        type = Number;
    }
    i++;
    return &i;
}

int fSum(double *anw)
{
    char op;
    double temp;
    if (fMulti(anw)) return 1;

    while ((op = *token) == '+' || op == '-')
    {
        getToken(expr);
        fMulti(&temp);
        switch (op)
        {
        case '+':
            *anw += temp;
            break;
        case '-':
            *anw -= temp;
            break;
        }
    }

    return 0;
}

int fMulti(double *anw)
{
    char op;
    double temp;
    if (fExp(anw)) return 1; //Ошибка

    while ((op = *token) == '*' || op == '/' || op == '%')
    {
        getToken(expr);
        if (fExp(&temp)) return 1; //Ошибка
        switch (op)
        {
        case '*':
            *anw *= temp;
            break;
        case '/':
            if (temp == 0.0)
            {
                error = Zero;
                return 1;
            }
            *anw /= temp;
            break;
        case '%':
            *anw = (int)*anw % (int)temp;
            break;
        }
    }

    return 0;
}

int fExp(double *anw)
{
    double temp;
    if (fUnary(anw)) return 1; //Ошибка

    while (*token == '^')
    {
        getToken(expr);
        if (fUnary(&temp)) return 1; //Ошибка
        *anw = pow(*anw, temp);
    }

    return 0;
}

int fUnary(double *anw)
{
    char op = 0;
    if (*token == '+' || *token == '-')
    {
        op = *token;
        getToken(expr);
    }
    if (fBrack(anw)) return 1; //Ошибка

    if (op == '-') *anw = -(*anw);

    return 0;
}

int fBrack(double *anw)
{
    if (*token == '(')
    {
        getToken(expr);
        fSum(anw);

        if (*token != ')')
        {
            error = Syntax;
            return 1;
        }
        getToken(expr);
    }
    else
        if (fAtom(anw)) return 1; //Ошибка

    return 0;
}

int fAtom(double *anw)
{
    if (type == Number)
    {
        *anw = atof(token);
        getToken(expr);
    }
    else
    {
        error = Syntax;
        return 1;
    }

    return 0;
}
