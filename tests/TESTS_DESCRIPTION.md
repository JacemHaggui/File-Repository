# Tests description

## testxxx series 

these tests are tested three times:

- once with both student client and server
- once with only student client
- once with only student server

the four xxx series (xxx is 0, 1, 2 or 3):

- test0 serie: basic tests, one single command
  (_big... = multipaquet, _err... = error expected)
- test1 serie: non classable short tests
- test2 serie: ability to continue (or not) after error
- test3 serie: quota tests

## tests with a buggy server

seedsrvX... serie tests client with buggy server
(X is the seed)

tests with "_CHCK" suffix cannot be automaticaly verified as
the expected output is underspecified

=> after running test, check them in tests/to_check
Generally, the minimal requirement is the output of the
expected error code

## tests with a buggy client

seedcliX... serie tests server with buggy client
(X is the seed)

these tests cannot be automaticaly verified as
the expected output is underspecified

=> after running test, check result in tests/buggyclient_tests
which contains a description of the expected result and the example
of the provided server behaviour.

## bad name test

last test is badname, with both server and client, then
with server alone. It tests whether an unauthorized file
is ignored by ls.
