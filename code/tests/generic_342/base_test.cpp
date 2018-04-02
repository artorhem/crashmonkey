#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <dirent.h>
#include <cstring>
#include <errno.h>

#include "BaseTestCase.h"
#include "../user_tools/api/workload.h"
#include "../user_tools/api/actions.h"

using fs_testing::tests::DataTestResult;
using fs_testing::user_tools::api::WriteData;
using fs_testing::user_tools::api::WriteDataMmap;
using fs_testing::user_tools::api::Checkpoint;
using std::string;

#define TEST_FILE_PERMS  ((mode_t) (S_IRWXU | S_IRWXG | S_IRWXO))

namespace fs_testing {
    namespace tests {
        
        
        class testName: public BaseTestCase {
            
            public:
            
            virtual int setup() override {
                
                return 0;
            }
            
            virtual int run() override {
                
                return 0;
            }
            
            virtual int check_test( unsigned int last_checkpoint, DataTestResult *test_result) override {


                struct stat stats_old;
                struct stat stats_new;
                const int stat_foo = stat(foo_path.c_str(), &stats_old);
                const int errno_foo = errno;
                const int stat_bar = stat(bar_path.c_str(), &stats_new);
                const int errno_bar = errno;

                // Neither stat found the file, it's gone...
                if (stat_foo < 0 && errno_foo == ENOENT &&
                    stat_bar < 0 && errno_bar == ENOENT) {
                  test_result->SetError(DataTestResult::kFileMissing); 
                  test_result->error_description = " : " + foo_path + " and " + bar_path+  " missing";
                  }

                // We renamed foo-> bar and created a new file foo. So old file foo's contents should be 
                // present in file bar. Else we have lost data present in old file foo during
                // rename.

                //We have lost file bar(contents of old foo)
                if(last_checkpoint ==1 && stat_bar < 0 && errno_bar == ENOENT && stats_old.st_size != 16384){
                  test_result->SetError(DataTestResult::kFileMissing); 
                  test_result->error_description = " : " + foo_path + " has new data " + bar_path + " missing";
                }

                //if bar is present, verify bar is the old foo 
                if(stat_bar == 0 && (stats_old.st_size != 4096 || stats_new.st_size != 16384)){
                  test_result->SetError(DataTestResult::kFileDataCorrupted); 
                  test_result->error_description = " : " + foo_path + " and " +bar_path + " has incorrect data";
                }
    

                return 0;
            }
            
            private:
            
        };
        
    }  // namespace tests
}  // namespace fs_testing

extern "C" fs_testing::tests::BaseTestCase *test_case_get_instance() {
    return new fs_testing::tests::testName;
}

extern "C" void test_case_delete_instance(fs_testing::tests::BaseTestCase *tc) {
    delete tc;
}
