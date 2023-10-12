// src\SmallImapsNotifier.cpp - entry point. Icon: https://icon-icons.com/???
// Copyright 2023 Alex0vSky (https://github.com/Alex0vSky)
#include "MsvcGenerated\stdafx.h"
#include "Main.h"
//int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, PSTR, int) {
int main(void) {
	HINSTANCE hInst = ::GetModuleHandle( NULL );
#ifdef _DEBUG
    HeapSetInformation( NULL, HeapEnableTerminationOnCorruption, NULL, NULL );
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
	UINT uExitCode = prj_sysw::SmallImapsNotifier::Main::run( hInst );
	return static_cast< int >( uExitCode );
}
