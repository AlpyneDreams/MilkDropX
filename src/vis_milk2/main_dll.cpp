
#include <windows.h>
#include "plugin.h"

#include "..\audio\common.h"

CPlugin g_plugin;
HINSTANCE api_orig_hinstance = nullptr;
_locale_t g_use_C_locale;
char keyMappings[8];

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
    api_orig_hinstance = hModule;
    return true;
}

IMilkDrop* GetMilkDrop()
{
    return &g_plugin;
}

// TODO: Only CoInit once etc.

class CMilkSystemAudio : public IMilkSystemAudio
{
    LoopbackCaptureThreadFunctionArguments threadArgs;
    CPrefs* m_prefs = nullptr;
    HANDLE hThread = NULL, hStopEvent = NULL;

    CPrefs& Prefs() {  return *m_prefs; }

public:
    CMilkSystemAudio::~CMilkSystemAudio()
    {
        StopThreads();
    }

    void DrainAudioBuf(unsigned char* pWaveL, unsigned char* pWaveR, int SamplesCount)
    {
        GetAudioBuf(pWaveL, pWaveR, SamplesCount);
    }

    int StartThreads()
    {
        if (hThread != NULL)
            return 0;

        HRESULT hr = S_OK;

        hr = CoInitialize(NULL);
        if (FAILED(hr)) {
            ERR(L"CoInitialize failed: hr = 0x%08x", hr);
            return -__LINE__;
        }
        CoUninitializeOnExit cuoe;

        // argc==1 No additional params. Output disabled.
        // argc==3 Two additional params. Output file enabled (32bit IEEE 754 FLOAT).
        // argc==4 Three additional params. Output file enabled (LITTLE ENDIAN PCM).
        int argc = 1;
        LPCWSTR argv[4] = { L"", L"--file", L"loopback-capture.wav", L"--int-16" };
        hr = S_OK;

        // parse command line
        m_prefs = new CPrefs(argc, argv, hr);
        if (FAILED(hr)) {
            ERR(L"CPrefs::CPrefs constructor failed: hr = 0x%08x", hr);
            return -__LINE__;
        }
        if (S_FALSE == hr) {
            // nothing to do
            return 0;
        }

        // create a "loopback capture has started" event
        HANDLE hStartedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (NULL == hStartedEvent) {
            ERR(L"CreateEvent failed: last error is %u", GetLastError());
            return -__LINE__;
        }
        CloseHandleOnExit closeStartedEvent(hStartedEvent);

        // create a "stop capturing now" event
        hStopEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (NULL == hStopEvent) {
            ERR(L"CreateEvent failed: last error is %u", GetLastError());
            return -__LINE__;
        }
        CloseHandleOnExit closeStopEvent(hStopEvent);

        // create arguments for loopback capture thread
        threadArgs = LoopbackCaptureThreadFunctionArguments();
        threadArgs.hr = E_UNEXPECTED; // thread will overwrite this
        threadArgs.pMMDevice = Prefs().m_pMMDevice;
        threadArgs.bInt16 = Prefs().m_bInt16;
        threadArgs.hFile = Prefs().m_hFile;
        threadArgs.hStartedEvent = hStartedEvent;
        threadArgs.hStopEvent = hStopEvent;
        threadArgs.nFrames = 0;

        hThread = CreateThread(
            NULL, 0,
            LoopbackCaptureThreadFunction, &threadArgs,
            0, NULL
        );
        if (NULL == hThread) {
            ERR(L"CreateThread failed: last error is %u", GetLastError());
            return -__LINE__;
        }
        CloseHandleOnExit closeThread(hThread);

        // wait for either capture to start or the thread to end
        HANDLE waitArray[2] = { hStartedEvent, hThread };
        DWORD dwWaitResult;
        dwWaitResult = WaitForMultipleObjects(
            ARRAYSIZE(waitArray), waitArray,
            FALSE, INFINITE
        );

        if (WAIT_OBJECT_0 + 1 == dwWaitResult) {
            ERR(L"Thread aborted before starting to loopback capture: hr = 0x%08x", threadArgs.hr);
            return -__LINE__;
        }

        if (WAIT_OBJECT_0 != dwWaitResult) {
            ERR(L"Unexpected WaitForMultipleObjects return value %u", dwWaitResult);
            return -__LINE__;
        }

        return 0;
    }

    int StopThreads()
    {
        if (hThread == NULL)
            return 0;

        //NEED TO STOP CAPTURE
        // at this point capture is running
        // wait for the user to press a key or for capture to error out
        {
            WaitForSingleObjectOnExit waitForThread(hThread);
            SetEventOnExit setStopEvent(hStopEvent);
            HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);

            if (INVALID_HANDLE_VALUE == hStdIn) {
                ERR(L"GetStdHandle returned INVALID_HANDLE_VALUE: last error is %u", GetLastError());
                return -__LINE__;
            }

            LOG(L"%s", L"Press Enter to quit...");

            HANDLE rhHandles[2] = { hThread, hStdIn };

            bool bKeepWaiting = true;
            while (bKeepWaiting) {

                DWORD dwWaitResult = WaitForMultipleObjects(2, rhHandles, FALSE, INFINITE);

                switch (dwWaitResult) {

                case WAIT_OBJECT_0: // hThread
                    ERR(L"%s", L"The thread terminated early - something bad happened");
                    bKeepWaiting = false;
                    break;

                case WAIT_OBJECT_0 + 1: // hStdIn
                                        // see if any of them was an Enter key-up event
                    /*INPUT_RECORD rInput[128];
                    DWORD nEvents;
                    if (!ReadConsoleInput(hStdIn, rInput, ARRAYSIZE(rInput), &nEvents)) {
                        ERR(L"ReadConsoleInput failed: last error is %u", GetLastError());
                        bKeepWaiting = false;
                    }
                    else {
                        for (DWORD i = 0; i < nEvents; i++) {
                            if (
                                KEY_EVENT == rInput[i].EventType &&
                                VK_RETURN == rInput[i].Event.KeyEvent.wVirtualKeyCode &&
                                !rInput[i].Event.KeyEvent.bKeyDown
                                ) {*/
                                LOG(L"%s", L"Stopping capture...");
                                bKeepWaiting = false;
                                break;
                    /*        }
                        }
                        // if none of them were Enter key-up events,
                        // continue waiting
                    }*/
                    break;

                default:
                    ERR(L"WaitForMultipleObjects returned unexpected value 0x%08x", dwWaitResult);
                    bKeepWaiting = false;
                    break;
                } // switch
            } // while
        } // naked scope

        // at this point the thread is definitely finished

        DWORD exitCode;
        if (!GetExitCodeThread(hThread, &exitCode)) {
            ERR(L"GetExitCodeThread failed: last error is %u", GetLastError());
            return -__LINE__;
        }

        if (0 != exitCode) {
            ERR(L"Loopback capture thread exit code is %u; expected 0", exitCode);
            return -__LINE__;
        }

        if (S_OK != threadArgs.hr) {
            ERR(L"Thread HRESULT is 0x%08x", threadArgs.hr);
            return -__LINE__;
        }

        if (NULL != Prefs().m_szFilename) {
            // everything went well... fixup the fact chunk in the file
            MMRESULT result = mmioClose(Prefs().m_hFile, 0);
            Prefs().m_hFile = NULL;
            if (MMSYSERR_NOERROR != result) {
                ERR(L"mmioClose failed: MMSYSERR = %u", result);
                return -__LINE__;
            }

            // reopen the file in read/write mode
            MMIOINFO mi = { 0 };
            Prefs().m_hFile = mmioOpenW(const_cast<LPWSTR>(Prefs().m_szFilename), &mi, MMIO_READWRITE);
            if (NULL == Prefs().m_hFile) {
                ERR(L"mmioOpen(\"%ls\", ...) failed. wErrorRet == %u", Prefs().m_szFilename, mi.wErrorRet);
                return -__LINE__;
            }

            // descend into the RIFF/WAVE chunk
            MMCKINFO ckRIFF = { 0 };
            ckRIFF.ckid = MAKEFOURCC('W', 'A', 'V', 'E'); // this is right for mmioDescend
            result = mmioDescend(Prefs().m_hFile, &ckRIFF, NULL, MMIO_FINDRIFF);
            if (MMSYSERR_NOERROR != result) {
                ERR(L"mmioDescend(\"WAVE\") failed: MMSYSERR = %u", result);
                return -__LINE__;
            }

            // descend into the fact chunk
            MMCKINFO ckFact = { 0 };
            ckFact.ckid = MAKEFOURCC('f', 'a', 'c', 't');
            result = mmioDescend(Prefs().m_hFile, &ckFact, &ckRIFF, MMIO_FINDCHUNK);
            if (MMSYSERR_NOERROR != result) {
                ERR(L"mmioDescend(\"fact\") failed: MMSYSERR = %u", result);
                return -__LINE__;
            }

            // write the correct data to the fact chunk
            LONG lBytesWritten = mmioWrite(
                Prefs().m_hFile,
                reinterpret_cast<PCHAR>(&threadArgs.nFrames),
                sizeof(threadArgs.nFrames)
            );
            if (lBytesWritten != sizeof(threadArgs.nFrames)) {
                ERR(L"Updating the fact chunk wrote %u bytes; expected %u", lBytesWritten, (UINT32)sizeof(threadArgs.nFrames));
                return -__LINE__;
            }

            // ascend out of the fact chunk
            result = mmioAscend(Prefs().m_hFile, &ckFact, 0);
            if (MMSYSERR_NOERROR != result) {
                ERR(L"mmioAscend(\"fact\") failed: MMSYSERR = %u", result);
                return -__LINE__;
            }
        }

        // let m_prefs' destructor call mmioClose
        delete m_prefs;

        return 0;
    }
};

static CMilkSystemAudio s_audio;

IMilkSystemAudio* GetMilkDropSystemAudioSink() { return &s_audio; }
