#include <rte_version.h>
#include <gtest/gtest.h>


TEST(test_rte, tst_version)
{
  const char* rte_version = rte_version();
  const char* argv1[] = 
}

int main(int argc,char**argv){

  testing::InitGoogleTest(&argc,argv);

  return RUN_ALL_TESTS();
}