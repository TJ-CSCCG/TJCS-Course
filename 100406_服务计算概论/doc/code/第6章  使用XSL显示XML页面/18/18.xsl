<?xml version="1.0" encoding="gb2312" ?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/TR/WD-xsl">
<xsl:template match="/">
  <html>
     <body>
       <h1><xsl:value-of select="root" /></h1>
       <h2>
          ������:<xsl:eval language="javascript">date()</xsl:eval>
          ����<xsl:eval>time()</xsl:eval>�򿪴�ҳ�档
       </h2>
     </body>
  </html>
  <xsl:script language="javascript">
     function date(){
        today=new Date();
        month=today.getMonth()+1;
        day=today.getDate();
        year=today.getYear();
        return year+"��"+month+"��"+day+"��";
     }
  </xsl:script>
  <xsl:script>
     function time(){
        now=new Date();
        hour=now.getHours();
        minute=now.getMinutes();
        second=now.getSeconds();
        return hour+"ʱ"+minute+"��"+second+"��";
     }
  </xsl:script>
</xsl:template>
</xsl:stylesheet>