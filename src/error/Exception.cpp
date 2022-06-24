#include "error/Exception.h"
#include "support/Util.h"

namespace netpp {
const char* InternalException::what() const noexcept
{
	return getErrorDescription(m_errCode).data();
}
}
