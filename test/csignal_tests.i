%module csignal_tests

%inline %{

#include <csignal.h>

#include "csignal_wrapper.h"

%}

%apply char       { CHAR  }
%apply char       { INT8  }
%apply short      { INT16 }
%apply int        { INT32 }
%apply long long  { INT64 }
%apply float      { FLOAT32 }
%apply double     { FLOAT64 }

%apply unsigned char      { UCHAR   }
%apply unsigned char      { UINT8   }
%apply unsigned short     { UINT16  }
%apply unsigned int       { UINT32  }
%apply unsigned long long { UINT64  }

%apply size_t { SIZE  }
%apply size_t { USIZE  }

%apply char *       { CHAR *  }
%apply char *       { INT8 *  }
%apply short *      { INT16 * }
%apply int *        { INT32 * }
%apply long long *  { INT64 * }
%apply float *      { FLOAT32 * }
%apply double *     { FLOAT64 * }

%apply unsigned char *      { UCHAR *   }
%apply unsigned char *      { UINT8 *   }
%apply unsigned short *     { UINT16 *  }
%apply unsigned int *       { UINT32 *  }
%apply unsigned long long * { UINT64 *  }

%apply unsigned char { CPC_BOOL }

%apply size_t * { SIZE *  }

%apply unsigned int { csignal_error_code }

%include <csignal.h>
%include <wav.h>
%include <spreading_code.h>
%include <gold_code.h>
%include <csignal_error_codes.h>
%include <fir_filter.h>
%include <kaiser_filter.h>
%include <bit_packer.h>
%include <bit_stream.h>

// These have to be included because we don't recursively parse headers
%include <types.h>
%include <cpcommon_error_codes.h>
%include <log_definitions.h>
%include <log_functions.h>

%include <cpointer.i>
%pointer_functions( double, doubleP )

%include <csignal_wrapper.h>
