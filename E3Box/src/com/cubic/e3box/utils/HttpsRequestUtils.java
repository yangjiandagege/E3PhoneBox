package com.cubic.e3box.utils;

import java.io.IOException;
import java.net.Socket;
import java.net.UnknownHostException;
import java.security.KeyManagementException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.UnrecoverableKeyException;
import java.util.UUID;
import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;
import org.apache.http.HttpResponse;
import org.apache.http.HttpVersion;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.HttpClient;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.conn.ClientConnectionManager;
import org.apache.http.conn.scheme.PlainSocketFactory;
import org.apache.http.conn.scheme.Scheme;
import org.apache.http.conn.scheme.SchemeRegistry;
import org.apache.http.conn.ssl.SSLSocketFactory;
import org.apache.http.entity.StringEntity;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.impl.conn.tsccm.ThreadSafeClientConnManager;
import org.apache.http.params.BasicHttpParams;
import org.apache.http.params.HttpParams;
import org.apache.http.params.HttpProtocolParams;
import org.apache.http.protocol.HTTP;
import org.apache.http.util.EntityUtils;
import org.json.JSONObject;

import android.os.StrictMode;
import android.os.SystemProperties;
import android.util.Log;

import com.cubic.e3box.Constants;

public class HttpsRequestUtils {
	private static final String TAG = "HttpsRequestUtils";
	private int mHttpOperateID = 0;
	private JSONObject mJsonReq = null;
	private String mUrl = null;
	/**************HTTPS REQUESTS************/
	public static final int HTTPS_REQ_CALL_All_GROUP_MEMBERS = 1;
	public static final int HTTPS_REQ_NOTIFY_MOTION_DETECTED = 2;
	public static final int HTTPS_REQ_NOTIFY_NEW_FIRMWARE = 3;
	public static final int HTTPS_REQ_NOTIFY_SECURITY_ALERT = 4;
	
	public HttpsRequestUtils(int httpOperateID,JSONObject jsonReq){
		mHttpOperateID = httpOperateID;
		mJsonReq = jsonReq;
	}
	
	public Boolean httpsPostData(){
		String strXApiKey =  SystemProperties.get(Constants.BLE_PROP_activate_api,Constants.BLE_DEFAULT_VALUE_NULL);
		String strGroup =  SystemProperties.get(Constants.BLE_PROP_activate_group,Constants.BLE_DEFAULT_VALUE_NULL);
		switch(mHttpOperateID){
		case HTTPS_REQ_CALL_All_GROUP_MEMBERS:
			mUrl = "https://api.test.e3phone.info/api/v1/groups/"+strGroup+"/call.json";
			break;
		case HTTPS_REQ_NOTIFY_MOTION_DETECTED:
			mUrl = "https://api.sandbox.e3phone.info/api/v1/"+strXApiKey+"/notify/motion_detect/";
			break;
		case HTTPS_REQ_NOTIFY_NEW_FIRMWARE:
			mUrl = "https://api.sandbox.e3phone.info/api/v1/groups/"+strXApiKey+"/notify/security_alerts";
			break;
		case HTTPS_REQ_NOTIFY_SECURITY_ALERT:
			mUrl = "https://api.sandbox.e3phone.info/api/v1/groups/"+strXApiKey+"/notifies/firmwares";
			break;
		default:
			break;
		}
		return httpsPostDataEx();
	}
	
	private Boolean httpsPostDataEx(){
	    StrictMode.ThreadPolicy policy=new StrictMode.ThreadPolicy.Builder().permitAll().build();
        StrictMode.setThreadPolicy(policy);
		try {
			String strXWSSE = getXWSSE();
			String strXApiKey = SystemProperties.get(Constants.BLE_PROP_activate_api,Constants.BLE_DEFAULT_VALUE_NULL);
			
			HttpClient httpClient = getNewHttpClient(); 
		    HttpPost httpPost = new HttpPost(mUrl);

		    httpPost.addHeader("X-WSSE", strXWSSE);
		    httpPost.addHeader("X-Api-Key", strXApiKey);

		    httpPost.setEntity(new StringEntity(mJsonReq.toString()));     

		    HttpResponse httpRsp = null;
		    httpRsp = httpClient.execute(httpPost);
		    int code = httpRsp.getStatusLine().getStatusCode();  
		    if (code == 200) {
		        String strHttpRsp = EntityUtils.toString(httpRsp.getEntity());       
		        JSONObject jsonRsp = new JSONObject(strHttpRsp);  
		        String strResult = jsonRsp.getString("result");  
		        String strReason = jsonRsp.getString("reason"); 
		        if(strResult.equals("201") && strReason.equals("Accepted")){
		        	return true;
		        }
		    }else{
		    	Log.e(TAG,"Code of https response is "+code);
		    	return false;
		    }
	    } catch (ClientProtocolException e) {
	    	e.printStackTrace();
	    } catch (IOException e) {
	    	e.printStackTrace();
	    } catch (Exception e) {
	    	e.printStackTrace();
	    }
		return false;
	}
	
	private String getXWSSE(){
		String strXWSSE = "UsernameToken ";
		String strUsernameInXWSSE = "Username=\"";
		String strDigestInXWSSE = "\",PasswordDigest=\"";
		String strNonceInXWSSE = "\",Nonce=\"";
		String strCreatedInXWSSE = "\",Created=\"";
		
		String strClientId = SystemProperties.get(Constants.BLE_PROP_islocalowner_client, Constants.BLE_DEFAULT_VALUE_NULL);
		strUsernameInXWSSE = strUsernameInXWSSE + strClientId;
		
		String strNonce = UUID.randomUUID().toString();
		strNonceInXWSSE = strNonceInXWSSE + strNonce;
		
		String strCreated = CommonUtils.getTimestamp();
		strCreatedInXWSSE = strCreatedInXWSSE + strCreated;
		
		String strApiPass = SystemProperties.get(Constants.BLE_PROP_activate_pass,Constants.BLE_DEFAULT_VALUE_NULL);
		String strDigest = CommonUtils.getDigestAndEncodeBase64(strNonce + strCreated + strApiPass);
		strDigestInXWSSE = strDigestInXWSSE + strDigest;
		
		strXWSSE = strXWSSE + strUsernameInXWSSE + strDigestInXWSSE + strNonceInXWSSE + strCreatedInXWSSE + "\"";
		
		return strXWSSE;
	}
	
	public HttpClient getNewHttpClient() {  
        try {
            KeyStore trustStore = KeyStore.getInstance(KeyStore.getDefaultType());  
            trustStore.load(null, null);  
            
            SSLSocketFactory sf = new SSLSocketFactoryEx(trustStore);  
            sf.setHostnameVerifier(SSLSocketFactory.ALLOW_ALL_HOSTNAME_VERIFIER);  
    
            HttpParams params = new BasicHttpParams();  
            HttpProtocolParams.setVersion(params, HttpVersion.HTTP_1_1);  
            HttpProtocolParams.setContentCharset(params, HTTP.UTF_8);  
    
            SchemeRegistry registry = new SchemeRegistry();  
            registry.register(new Scheme("http", PlainSocketFactory.getSocketFactory(), 80));  
            registry.register(new Scheme("https", sf, 443));  
    
            ClientConnectionManager ccm = new ThreadSafeClientConnManager(params, registry);  

            return new DefaultHttpClient(ccm, params);  
        } catch (Exception e) {  
            return new DefaultHttpClient();  
        }  
    }
	
	private class SSLSocketFactoryEx extends SSLSocketFactory {
	        SSLContext sslContext = SSLContext.getInstance("TLS");
	        
	        public SSLSocketFactoryEx(KeyStore truststore) 
	                        throws NoSuchAlgorithmException, KeyManagementException,
	                        KeyStoreException, UnrecoverableKeyException {
	            super(truststore);
	                
	            TrustManager tm = new X509TrustManager() {
	            public java.security.cert.X509Certificate[] getAcceptedIssuers() {return null;}  
	    
	            @Override  
	            public void checkClientTrusted(
	                            java.security.cert.X509Certificate[] chain, String authType)
	                                            throws java.security.cert.CertificateException {}  
	    
	            @Override  
	            public void checkServerTrusted(
	                            java.security.cert.X509Certificate[] chain, String authType)
	                                            throws java.security.cert.CertificateException {}
	        };  
	        sslContext.init(null, new TrustManager[] { tm }, null);  
	    }
	    
	    @Override  
	    public Socket createSocket(Socket socket, String host, int port,boolean autoClose) throws IOException, UnknownHostException {  
	            return sslContext.getSocketFactory().createSocket(socket, host, port,autoClose);  
	    }  
	    
	    @Override  
	    public Socket createSocket() throws IOException {  
	        return sslContext.getSocketFactory().createSocket();  
	    }  
	}
}
