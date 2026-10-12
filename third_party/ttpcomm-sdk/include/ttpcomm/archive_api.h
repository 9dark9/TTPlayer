#pragma once
#include <windows.h>
#include <objidl.h>
#include <stdint.h>

/* Additive, read-only archive service. Strings are UTF-16. Entry names are
   borrowed only during the visitor call. Returned IStreams own their storage;
   Release them before unloading the DLL. No CRT/STL ownership crosses the ABI.
   Callbacks return FALSE to cancel and must not reenter this service. */
#define TTPCOMM_ARCHIVE_ABI 1u
#define TTPCOMM_ARCHIVE_RAR4 1u
#define TTPCOMM_ARCHIVE_RAR5 2u
#define TTPCOMM_ARCHIVE_REQUIRED 3u
#define TTPCOMM_ARCHIVE_DIRECTORY 1u
#define TTPCOMM_ARCHIVE_ENCRYPTED 2u
#define TTPCOMM_ARCHIVE_SOLID 4u
#define TTPCOMM_ARCHIVE_LINK 8u
#define TTPCOMM_ARCHIVE_SPLIT 16u
#define TTPCOMM_ARCHIVE_PASSWORD ((HRESULT)0x8004A001L)
#define TTPCOMM_ARCHIVE_BAD_PASSWORD ((HRESULT)0x8004A002L)
#define TTPCOMM_ARCHIVE_MISSING_VOLUME ((HRESULT)0x8004A003L)
#define TTPCOMM_ARCHIVE_DICTIONARY_LIMIT ((HRESULT)0x8004A004L)
#define TTPCOMM_ARCHIVE_UNSUPPORTED ((HRESULT)0x8004A005L)

#pragma pack(push, 8)
typedef struct TtpCommArchiveOptions {
    uint32_t size;
    uint32_t dictionary_limit_kib; /* 0: 256 MiB; hard ceiling 512 MiB. */
    uint64_t member_limit_bytes;   /* 0: 8 GiB; applies before/during decode. */
    uint32_t memory_limit_bytes;   /* 0: 16 MiB; larger streams use delete-on-close files. */
    uint32_t entry_limit;          /* 0: 100000. */
    const wchar_t* password;       /* Optional; never persisted by the service. */
    BOOL (__cdecl *progress)(void* context, uint64_t completed, uint64_t total);
    void* context;
} TtpCommArchiveOptions;

typedef struct TtpCommArchiveEntry {
    uint32_t size, index, flags, dictionary_kib;
    uint64_t unpacked_bytes, packed_bytes;
    const wchar_t* name;
} TtpCommArchiveEntry;

typedef BOOL (__cdecl *TtpCommArchiveVisitor)(void*, const TtpCommArchiveEntry*);
typedef struct TtpCommArchiveApi {
    uint32_t size, abi_version, capabilities, reserved;
    HRESULT (__cdecl *enumerate)(const wchar_t*, const TtpCommArchiveOptions*,
                                TtpCommArchiveVisitor, void*);
    HRESULT (__cdecl *open_member)(const wchar_t*, const wchar_t*,
                                  const TtpCommArchiveOptions*, IStream**);
} TtpCommArchiveApi;
#pragma pack(pop)
typedef BOOL (__cdecl *TtpCommQueryArchiveFn)(uint32_t abi, TtpCommArchiveApi*);

/* ABI 2 is available at ttpcomm_query_archive_stream (ordinal 503), and at
   ttpcomm_query_archive with ABI 2. ABI 1 stays synchronous and unchanged.
   Each stream owns its cancellation callback. It must outlive that stream
   and its clones; only caller threads poll it, never the cached producer.
   Release all streams and call clear_cache before unloading the DLL. */
#define TTPCOMM_ARCHIVE_STREAM_ABI 2u
#define TTPCOMM_ARCHIVE_ZIP 4u
#define TTPCOMM_ARCHIVE_PROGRESSIVE 8u
#define TTPCOMM_ARCHIVE_STREAM_REQUIRED 15u
#define TTPCOMM_ARCHIVE_CACHE_CURRENT 1u
#pragma pack(push, 8)
typedef struct TtpCommArchiveStreamOptions {
    uint32_t size, flags;
    TtpCommArchiveOptions limits; /* progress/context unused; use canceled. */
    BOOL (__cdecl *canceled)(void* context);
    void* context;
    uint32_t timeout_ms; /* 0: 30000 per wait. */
} TtpCommArchiveStreamOptions;
typedef struct TtpCommArchiveStreamStatus {
    uint32_t size, complete;
    uint64_t completed_bytes, total_bytes, task_id;
    HRESULT result; /* E_PENDING until verified; failure is terminal. */
} TtpCommArchiveStreamStatus;
typedef struct TtpCommArchiveStreamApi {
    uint32_t size, abi_version, capabilities, reserved;
    HRESULT (__cdecl *enumerate)(const wchar_t*, const TtpCommArchiveOptions*,
                                TtpCommArchiveVisitor, void*);
    HRESULT (__cdecl *open_member)(const wchar_t*, const wchar_t*,
                                  const TtpCommArchiveStreamOptions*, IStream**);
    HRESULT (__cdecl *status)(IStream*, TtpCommArchiveStreamStatus*);
    void (__cdecl *clear_cache)(void); /* Active streams remain valid. */
} TtpCommArchiveStreamApi;
#pragma pack(pop)
typedef BOOL (__cdecl *TtpCommQueryArchiveStreamFn)(uint32_t, TtpCommArchiveStreamApi*);
