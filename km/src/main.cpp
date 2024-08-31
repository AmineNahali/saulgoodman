#include<ntifs.h>

extern "C" {
	NTKERNELAPI NTSTATUS IoCreateDriver(PUNICODE_STRING DriverName,
		                                PDRIVER_INITIALIZE InitializationFunction);

	NTKERNELAPI NTSTATUS MmCopyVirtualMemory(PEPROCESS SourceProcess, PVOID SourceAdress,
		                                     PEPROCESS TargetProcess, PVOID TargetAdress,
		                                     SIZE_T BufferSize, KPROCESSOR_MODE PreviousMode,
		                                     SIZE_T ReturnSize);
}


namespace driver {
	namespace codes {
		//setup the driver
		constexpr ULONG attach =
			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x696, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
		//read process memory
		constexpr ULONG read =
			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x697, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
		//write to process memory
		constexpr ULONG write =
			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x698, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
	}

	struct Request {
		HANDLE process_id;
		PVOID target;
		PVOID buffer;
		SIZE_T size;
		SIZE_T return_size;
	};

	NTSTATUS create(PDEVICE_OBJECT device_object, PIRP irp) {
		UNREFERENCED_PARAMETER(device_object);
		IoCompleteRequest(irp, IO_NO_INCREMENT);
		return irp->IoStatus.Status;
	}
	NTSTATUS close(PDEVICE_OBJECT device_object, PIRP irp) {
		UNREFERENCED_PARAMETER(device_object);
		IoCompleteRequest(irp, IO_NO_INCREMENT);
		return irp->IoStatus.Status;
	}
	//Note:Todo
	NTSTATUS device_control(PDEVICE_OBJECT device_object, PIRP irp) {
		UNREFERENCED_PARAMETER(device_object);     //
		IoCompleteRequest(irp, IO_NO_INCREMENT);   //
		return irp->IoStatus.Status;               //
	}
}


void debug_print(PCSTR text) {

#ifndef DEBUG
	UNREFERENCED_PARAMETER(text);
#endif // !DEBUG

	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, text));
}


NTSTATUS DriverMain(PDRIVER_OBJECT driver_object, PUNICODE_STRING registry_path) {
	UNREFERENCED_PARAMETER(registry_path);

	//create driver device object
	UNICODE_STRING device_name = {};
	RtlInitUnicodeString(&device_name, L"\\Device\\RootBeerDevice");
	
	PDEVICE_OBJECT device_object = nullptr;
	NTSTATUS status = IoCreateDevice(driver_object,
		                             0,
		                             &device_name,
		                             FILE_DEVICE_UNKNOWN,
		                             FILE_DEVICE_SECURE_OPEN,
		                             FALSE, &device_object);
	if (status != STATUS_SUCCESS) {
		debug_print("[-] Failed to create driver device.\n");
	}
	debug_print("[+] Driver device successfully created.\n");

	//establish symbolic link
	UNICODE_STRING symbolic_link = {};
	RtlInitUnicodeString(&symbolic_link, L"\\DosDevices\\RootBeer");

	status = IoCreateSymbolicLink(&symbolic_link, &device_name);

	if (status != STATUS_SUCCESS) {
		debug_print("[-] Failed to establish symbolic link.\n");
	}
	debug_print("[+] Symbolic link successfully established.\n");

	//setup communication between um & km
	SetFlag(device_object->Flags, DO_BUFFERED_IO);
	driver_object->MajorFunction[IRP_MJ_CREATE] = driver::create;

	return STATUS_SUCCESS;
}


NTSTATUS DriverEntry() {

	debug_print("[+] meh driver is runnin\n");

	UNICODE_STRING driver_name = {};
	RtlInitUnicodeString(&driver_name, L"\\Driver\\RootBeer");

	return IoCreateDriver(&driver_name,&DriverMain);
}
