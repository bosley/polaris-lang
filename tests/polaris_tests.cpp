
#include "polaris/polaris.hpp"
#include <vector>

#include <CppUTest/TestHarness.h>

TEST_GROUP(polaris_tests){};

TEST(polaris_tests, all)
{
  struct test_case_t {
    std::string input;
    std::string expected_output;
  };

  std::vector<test_case_t> tests = {
    {"(quote (testing 1 (2.0) -3.14e159))", "(testing 1 (2.0) -3.14e159)"},
    {"(+ 2 2)", "4"},
    {"(+ (* 2 100) (* 1 10))", "210"},
    {"(if (> 6 5) (+ 1 1) (+ 2 2))", "2"},
    {"(if (< 6 5) (+ 1 1) (+ 2 2))", "4"},
    {"(define x 3)", "3"},
    {"x", "3"},
    {"(+ x x)", "6"},
    {"(begin (define x 1) (set! x (+ x 1)) (+ x 1))", "3"},
    {"((lambda (x) (+ x x)) 5)", "10"},
    {"(define twice (lambda (x) (* 2 x)))", "<Lambda>"},
    {"(twice 5)", "10"},
    {"(define compose (lambda (f g) (lambda (x) (f (g x)))))", "<Lambda>"},
    {"((compose list twice) 5)", "(10)"},
    {"(define repeat (lambda (f) (compose f f)))", "<Lambda>"},
    {"((repeat twice) 5)", "20"},
    {"((repeat (repeat twice)) 5)", "80"},
    {"(define fact (lambda (n) (if (<= n 1) 1 (* n (fact (- n 1))))))", "<Lambda>"},
    {"(fact 3)", "6"},
    {"(fact 12)", "479001600"},
    {"(define abs (lambda (n) ((if (> n 0) + -) 0 n)))", "<Lambda>"},
    {"(list (abs -3) (abs 0) (abs 3))", "(3 0 3)"},
    {"(define combine (lambda (f)"
             "(lambda (x y)"
                "(if (null? x) (quote ())"
                "(f (list (car x) (car y))"
                "((combine f) (cdr x) (cdr y)))))))", "<Lambda>"},
    {"(define zip (combine cons))", "<Lambda>"},
    {"(zip (list 1 2 3 4) (list 5 6 7 8))", "((1 5) (2 6) (3 7) (4 8))"},
    {"(define riff-shuffle (lambda (deck) (begin"
            "(define take (lambda (n seq) (if (<= n 0) (quote ()) (cons (car seq) (take (- n 1) (cdr seq))))))"
            "(define drop (lambda (n seq) (if (<= n 0) seq (drop (- n 1) (cdr seq)))))"
            "(define mid (lambda (seq) (/ (length seq) 2)))"
            "((combine append) (take (mid deck) deck) (drop (mid deck) deck)))))", "<Lambda>"},
    {"(riff-shuffle (list 1 2 3 4 5 6 7 8))", "(1 5 2 6 3 7 4 8)"},
    {"((repeat riff-shuffle) (list 1 2 3 4 5 6 7 8))",  "(1 3 5 7 2 4 6 8)"},
    {"(riff-shuffle (riff-shuffle (riff-shuffle (list 1 2 3 4 5 6 7 8))))", "(1 2 3 4 5 6 7 8)"},
  };

  polaris::environment_c test_env;
  polaris::add_globals(test_env);

  for(auto &tc : tests) {
    auto result = polaris::to_string(polaris::eval(polaris::read(tc.input), &test_env));
    CHECK_EQUAL_TEXT(result, tc.expected_output, "Output did not meet expectations");
  }

}