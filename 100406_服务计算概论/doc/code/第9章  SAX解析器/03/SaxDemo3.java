import org.xml.sax.helpers.DefaultHandler;
import org.xml.sax.*;
import javax.xml.parsers.*;

public class SaxDemo3 {
    public static void main(String[] args) {
        try {
            SAXParserFactory factory = SAXParserFactory.newInstance();
            SAXParser saxParser = factory.newSAXParser();
            MyHandler myHandler = new MyHandler();
            saxParser.parse("03.xml",myHandler);
        }  catch (Exception e) {
            e.printStackTrace();
        }

    }
}
class MyHandler extends DefaultHandler{
    long startTime ;
    int n=0;
    public void startDocument() throws SAXException {
        startTime = System.currentTimeMillis();
        System.out.println("��ʼ����MXL�ļ���");
    }

    public void endDocument() throws SAXException {
        System.out.println("�����ļ�����");
        System.out.println("�����ļ�����ʱ�䣺"+(System.currentTimeMillis()-startTime)+"����");
    }

    public void processingInstruction(String target, String data) throws SAXException {
        System.out.println("����ָ���¼�"+(++n)+":");
        System.out.println("�ļ��а�����ָ��"+n+":");
        System.out.println("<?"+target+" "+data+" ?>");
	if(target.equals("displayend")){
		System.out.println("XML�ļ�����");
	}
    }
}
