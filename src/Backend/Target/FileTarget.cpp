#include "FileTarget.h"

#include "Backend/ISourceRenderer.h"

using namespace Backend;

FileTarget::FileTarget( const char* filePath )
    : m_filePath( filePath )
{
}

FileTarget::~FileTarget()
{
    delete m_filePath;
}

void
FileTarget::configure( Backend::ISourceRenderer *renderer )
{
    renderer->setOutputFile( m_filePath );
}
