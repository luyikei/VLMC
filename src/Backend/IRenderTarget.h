#ifndef IRENDERTARGET_H
#define IRENDERTARGET_H


namespace Backend
{
    class ISourceRenderer;

    class IRenderTarget
    {
    public:
        virtual ~IRenderTarget() = default;
        virtual void configure( ISourceRenderer* renderer ) = 0;
    };
}

#endif // IRENDERTARGET_H

