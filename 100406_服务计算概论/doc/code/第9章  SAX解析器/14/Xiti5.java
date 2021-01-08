import org.xml.sax.helpers.DefaultHandler;
import org.xml.sax.helpers.XMLFilterImpl;
import org.xml.sax.*;

import javax.xml.parsers.*;

public class Xiti5 {
    public static void main(String[] args) {
        try {
            SAXParserFactory factory = SAXParserFactory.newInstance();
            SAXParser saxParser = factory.newSAXParser();
            XMLReader reader = saxParser.getXMLReader();
            Myfilter myfilter = new Myfilter();
            myfilter.setParent(reader);
            MyHandler myHandler = new MyHandler();
            myfilter.setContentHandler(myHandler);
            myfilter.parse("xiti5.xml");
        } catch (Exception e) {
            e.printStackTrace();
        }

    }
}

class MyHandler extends DefaultHandler {
    public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
        System.out.print("<"+qName+">");
    }

    public void endElement(String uri, String localName, String qName) throws SAXException {
        System.out.println("</"+qName+">");
    }

    public void characters(char ch[], int start, int length) throws SAXException {
         System.out.print(new String(ch, start, length));

    }
}

class Myfilter extends XMLFilterImpl {
    boolean flag = false;
    public void startElement(String uri, String localName, String qName, Attributes atts) throws SAXException {
        if (!qName.equals("age")) {
           super.startElement(uri, localName, qName, atts);
           flag=true;
        }
    }

    public void endElement(String uri, String localName, String qName) throws SAXException {
       if (!qName.equals("age")) {
           super.endElement(uri, localName, qName);
        }
    }

    public void characters(char ch[], int start, int length) throws SAXException {
        if (flag) {
            super.characters(ch, start, length);
            flag = false;
        }
    }
}

