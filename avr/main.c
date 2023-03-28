#include "APP/com_project_master.h"
#include "APP/com_project_slave.h"
int main()
{

    // project_master_init();
    // project_master_app();

    project_slave_init();
    project_slave_app();
    return 0;
}