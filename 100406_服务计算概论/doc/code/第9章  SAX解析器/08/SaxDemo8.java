import org.xml.sax.helpers.DefaultHandler;
import org.xml.sax.*;
import javax.xml.parsers.*;
import java.io.IOException;

public class SaxDemo8 {
    public static void main(String[] args) {
        try {
            SAXParserFactory factory = SAXParserFactory.newInstance();
            SAXParser saxParser = factory.newSAXParser();
            MyHandler myHandler = new MyHandler();
            saxParser.parse("08.xml", myHandler);
        } catch (Exception e) {
            e.printStackTrace();
        }

    }
}

class MyHandler extends DefaultHandler {
    int n=0,m=0;
    public void startDocument() throws SAXException {
        System.out.println("��ʼ����MXL�ļ���");
    }

    public void endDocument() throws SAXException {
        System.out.println("�����ļ�����");
        System.out.println("������������"+n+"��ʵ��(����DOCTYPE������"+m+"�������Ե�ʵ�塣)");
    }

    public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
        System.out.print("<"+qName+">");
    }

    public void endElement(String uri, String localName, String qName) throws SAXException {
        System.out.print("</"+qName+">");
    }

    public void characters(char ch[], int start, int length) throws SAXException {
        System.out.print(new String(ch,start,length));
    }

    public InputSource resolveEntity(String publicId, String systemId) throws IOException, SAXException {
        System.out.println("��"+(++n)+"��ʵ�壺");
        System.out.println("publicId="+publicId+",systemId="+systemId);
        return null;
    }

    public void skippedEntity(String name) throws SAXException {
        m++;
        System.out.println("��"+(++n)+"��ʵ��,�����Ե�ʵ�壺"+name);
    }

}

