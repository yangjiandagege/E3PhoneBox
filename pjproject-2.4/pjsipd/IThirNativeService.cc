#ifndef _ITHIR_NATIVE_SERVICE_CC_
#define _ITHIR_NATIVE_SERVICE_CC_ 1

#include <stdlib.h>
#include <binder/IInterface.h>
#include <binder/IBinder.h>
#include <binder/Parcel.h>

using namespace android;

#define THIR_NATIVE_SERVICE_DESCRIPTOR "me.thirchina.SipNaticeService"

// interface
class IThirNativeService : public IInterface
{
public:
	enum {
        TRANSACTION_sendCmd = IBinder::FIRST_CALL_TRANSACTION + 0,
        TRANSACTION_sendData = IBinder::FIRST_CALL_TRANSACTION + 1,
        TRANSACTION_regBinder = IBinder::FIRST_CALL_TRANSACTION + 2,
    };

	virtual int32_t sendCmd(int32_t code, const char16_t* info) = 0;
	virtual int32_t sendData(int32_t size, const void* data) = 0;
	virtual int32_t regBinder(sp<IBinder> &binder) = 0;

	DECLARE_META_INTERFACE(ThirNativeService);
};

// client-end
class BpThirNativeService : public BpInterface<IThirNativeService>
{
public:
	BpThirNativeService(const sp<IBinder>& impl) : BpInterface<IThirNativeService>(impl)
	{};

    virtual int32_t sendCmd(int32_t code, const char16_t* info)
    {
    	Parcel data, reply;
    	data.writeInterfaceToken(IThirNativeService::getInterfaceDescriptor());
    	data.writeInt32(code);
    	data.writeString16(String16(info));
    	remote()->transact(TRANSACTION_sendCmd, data, &reply);
    	return reply.readInt32();
    };
    
    virtual int32_t sendData(int32_t size, const void* p_data)
    {
    	Parcel data, reply;
    	data.writeInterfaceToken(IThirNativeService::getInterfaceDescriptor());
    	data.writeInt32(size);
    	data.writeInt32(size);
    	data.write(p_data, size);
    	remote()->transact(TRANSACTION_sendData, data, &reply);
    	return reply.readInt32();
    };
    
    virtual int32_t regBinder(sp<IBinder> &binder)
    {
    	Parcel data, reply;
    	data.writeInterfaceToken(IThirNativeService::getInterfaceDescriptor());
    	data.writeStrongBinder(binder);
    	remote()->transact(TRANSACTION_regBinder, data, &reply);
    	return reply.readInt32();
    };
};
IMPLEMENT_META_INTERFACE(ThirNativeService, THIR_NATIVE_SERVICE_DESCRIPTOR);


// Server-end
class BnThirNativeService : public BnInterface<IThirNativeService> {
public:
    virtual status_t onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags = 0)
    {
    	switch(code)
    	{
		case TRANSACTION_sendCmd:
			CHECK_INTERFACE(IThirNativeService, data, reply);
			if( reply != NULL )
			{
				int32_t code = data.readInt32();
				String16 info = data.readString16();
				int32_t ret = this->sendCmd(code, (const char16_t*)info);
				reply->writeInt32(ret);
			}
			return NO_ERROR;
		
		case TRANSACTION_sendData:
			CHECK_INTERFACE(IThirNativeService, data, reply);
			if( reply != NULL )
			{
				int32_t size = data.readInt32();
				unsigned char* buf = (unsigned char*)malloc(size);
				if( buf == NULL )
				{
					reply->writeInt32(-1);
					return NO_ERROR;
				}

				if( 0 != data.read(buf, size) )
				{
					free(buf);
					reply->writeInt32(-1);
					return NO_ERROR;
				}

				int32_t ret = this->sendData(size, buf);
				free(buf);
				reply->writeInt32(ret);
			}
			return NO_ERROR;

		case TRANSACTION_regBinder:
			CHECK_INTERFACE(IThirNativeService, data, reply);
			if( reply != NULL )
			{
				sp<IBinder> binder = data.readStrongBinder();
				int32_t ret = this->regBinder(binder);
				reply->writeInt32(ret);
			}
			return NO_ERROR;

		default:
			return BBinder::onTransact(code, data, reply, flags);
    	}
    };
};

#endif //_ITHIR_NATIVE_SERVICE_CC_
