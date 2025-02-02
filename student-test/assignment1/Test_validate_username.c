#include "unity.h"
#include <stdbool.h>
#include <stdlib.h>
#include "../../examples/autotest-validate/autotest-validate.h"
#include "../../assignment-autotest/test/assignment1/username-from-conf-file.h"

/**
* This function should:
*   1) Call the my_username() function in autotest-validate.c to get your hard coded username.
*   2) Obtain the value returned from function malloc_username_from_conf_file() in username-from-conf-file.h within
*       the assignment autotest submodule at assignment-autotest/test/assignment1/
*   3) Use unity assertion TEST_ASSERT_EQUAL_STRING_MESSAGE to verify the two strings are equal.  See
*       the [unity assertion reference](https://github.com/ThrowTheSwitch/Unity/blob/master/docs/UnityAssertionsReference.md)
*/
void test_validate_my_username()
{
    const char *hc_name=my_username();
    char *cfile_name;
    /**
     * TODO: Replace the line below with your code here as described above to verify your /conf/username.txt 
     * config file and my_username() functions are setup properly
     */
    
    cfile_name=malloc_username_from_conf_file();
    /*bool test; 
    if (*cfile_name == *hc_name)
    {
    test = true;
    }
    else
    {
    test = false;
    }*/
    TEST_ASSERT_EQUAL_STRING(hc_name, cfile_name);
    //TEST_ASSERT_TRUE_MESSAGE(test,"AESD students, please fix me!");
}
