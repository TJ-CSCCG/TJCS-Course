import org.xml.sax.helpers.DefaultHandler;
import org.xml.sax.*;
import javax.xml.parsers.*;

public class SaxDemo4 {
    public static void main(String[] args) {
        try {
            SAXParserFactory factory = SAXParserFactory.newInstance();
            factory.setNamespaceAware(true);
            SAXParser saxParser = factory.newSAXParser();
            MyHandler myHandler = new MyHandler();
            saxParser.parse("04.xml", myHandler);
        } catch (Exception e) {
            e.printStackTrace();
        }

    }
}

class MyHandler extends DefaultHandler {
    long startTime;
    int n=0;
    public void startDocument() throws SAXException {
        startTime = System.currentTimeMillis();
        System.out.println("开始解析MXL文件：");
    }

    public void endDocument() throws SAXException {
        System.out.println("解析文件结束");
        System.out.println("解析文件所用时间：" + (System.currentTimeMillis() - startTime) + "毫秒");
    }

    public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
        System.out.println((++n)+".\""+localName+"\"标记，标记开始事件");
        System.out.print("  标记名称：" + localName + "，");
        System.out.print("名称空间：" + uri + "，");
        System.out.println("标记名称：" + qName + "，");
        if (attributes.getLength() > 0) {
            System.out.print("  包含属性：");
            for (int i = 0; i < attributes.getLength(); i++) {
                System.out.print( attributes.getLocalName(i));
                System.out.println(" = " + attributes.getValue(i)+";");
            }
        }else{
            System.out.println("  包含属性：null");
        }
    }

    public void endElement(String uri, String localName, String qName) throws SAXException {
        System.out.println("\""+localName+"\"标记结束事件");
    }
}
