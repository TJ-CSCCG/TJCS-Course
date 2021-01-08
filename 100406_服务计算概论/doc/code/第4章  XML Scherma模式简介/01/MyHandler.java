import org.xml.sax.helpers.DefaultHandler;
import org.xml.sax.SAXParseException;
import org.xml.sax.SAXException;

public class MyHandler extends DefaultHandler {
    String em=null;
    public void error(SAXParseException e)throws SAXException{
        em=e.getMessage();
        System.out.println("Ò»°ã´íÎó"+em);
    }
    public void fataError(SAXParseException e)throws SAXException{
        em=e.getMessage();
        System.out.println("ÖÂÃü´íÎó"+em);
    }
}
