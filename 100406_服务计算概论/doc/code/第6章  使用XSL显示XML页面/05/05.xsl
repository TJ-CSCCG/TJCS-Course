<?xml version="1.0" encoding="gb2312" ?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/TR/WD-xsl">
  <xsl:template math="/">
	<html>
	<body>
		<center>
		<table border="1" width="300" >
		<tr>
			<td align="center">lxfs</td>
		</tr>
		<xsl:for-each select="message/persion">
		<tr>
			<td align="center"><xsl:value-of select="lxfs/*" /></td>
		</tr>
		</xsl:for-each>
		</table>
		</center>
	</body>
	</html>
  </xsl:template>
</xsl:stylesheet>