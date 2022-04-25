// #include <vld.h>
#include "kinetic_model.h"
#include "args.h"
#include "image_vectorization_console.h"

#ifdef _WIN32
#include <Windows.h>
#include <psapi.h>
#else
#include <unistd.h>
#endif
#include <stdio.h>


#ifdef _WIN32
#define PSAPI_VERSION 1

// To ensure correct resolution of symbols, add Psapi.lib to TARGETLIBS
// and compile with -DPSAPI_VERSION=1

void PrintMemoryInfo( DWORD processID )
{
    HANDLE hProcess;
    PROCESS_MEMORY_COUNTERS pmc;

    // Print the process identifier.

    printf( "\nProcess ID: %u\n", processID );

    // Print information about the memory usage of the process.

    hProcess = OpenProcess(  PROCESS_QUERY_INFORMATION |
                                    PROCESS_VM_READ,
                                    FALSE, processID );
    if (NULL == hProcess)
        return;

    if ( GetProcessMemoryInfo( hProcess, &pmc, sizeof(pmc)) )
    {
        printf( "\tPageFaultCount: %i\n", int(pmc.PageFaultCount) );
        printf( "\tPeakWorkingSetSize: %lf\n", double(pmc.PeakWorkingSetSize) / (1024 * 1024));
        printf( "\tWorkingSetSize: %i\n", int(pmc.WorkingSetSize) / (1024 * 1024) );
        printf( "\tQuotaPeakPagedPoolUsage: %i\n", int(pmc.QuotaPeakPagedPoolUsage) / (1024 * 1024) );
        printf( "\tQuotaPagedPoolUsage: %i\n", int(pmc.QuotaPagedPoolUsage) / (1024 * 1024) );
        printf( "\tQuotaPeakNonPagedPoolUsage: %i\n", int(pmc.QuotaPeakNonPagedPoolUsage) / (1024 * 1024) );
        printf( "\tQuotaNonPagedPoolUsage: %i\n", int(pmc.QuotaNonPagedPoolUsage) / (1024 * 1024) );
        printf( "\tPagefileUsage: %i\n", int(pmc.PagefileUsage) / (1024 * 1024) ); 
        printf( "\tPeakPagefileUsage: %i\n", int(pmc.PeakPagefileUsage) / (1024 * 1024) );
    }

    CloseHandle( hProcess );
}
#endif

int main(int argc, char *argv[])
{
	GDALAllRegister();
	Kinetic_Model* model = new Kinetic_Model();
	if (!Args::parse(argc, argv, model->params)) {
		return 1;
	}
	std::cout << "Enters process" << std::endl;

	Partition_Refine::process(model);
	delete model;

    return 0;

}

