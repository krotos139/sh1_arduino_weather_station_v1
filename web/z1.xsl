<?xml version='1.0' encoding='UTF-8'?>
<xsl:stylesheet version='1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>

<xsl:template match='/'>
  <html>
  <head>
	  <title>Weather station</title>
	  <style>
	   .z1 {
			font-family:Arial, Helvetica, sans-serif;
			color:#666;
			font-size:12px;
			text-shadow: 1px 1px 0px #fff;
			background:#eaebec;
			margin:20px;
			border:#ccc 1px solid;
			border-collapse:separate; 
			border-radius:3px;
			box-shadow: 0 1px 2px #d1d1d1;
	   }
	   .z1 th {
			font-weight:bold;
			padding:15px;
			border-bottom:1px solid #e0e0e0;
			background: #ededed;
			background: linear-gradient(to top,  #ededed,  #ebebeb);
	   }
	   .z1 td {
			padding:10px;
			background: #f2f2f2;
			background: linear-gradient(to top,  #f2f2f2,  #f0f0f0);  
	   }
		.z1 tr:hover td{
			background: #aaaaaa;
			background: linear-gradient(to top, #f2f2f2,  #e0e0e0);  
		}
	  </style>
  </head>
  <body>
  <h2>Weather station</h2>
    <table class='z1'>
      <tr>
        <th>Property</th>
        <th>Value</th>
      </tr>
      <tr>
        <td> Temperature </td>
        <td><xsl:value-of select='response/temperature/celsius'/> C</td>
      </tr>
      <tr>
        <td> Humidity </td>
        <td><xsl:value-of select='response/humidity/percentage'/> %</td>
      </tr>
      <tr>
        <td> Pressure </td>
        <td><xsl:value-of select='response/pressure/mmHg'/> mm.Hg</td>
      </tr>
      <tr>
        <td> Illuminance </td>
        <td><xsl:value-of select='response/illuminance/lx'/> lx</td>
      </tr>
    </table>
	<h2>Termosensor</h2>
    <table class='z1'>
      <tr>
        <th>Sensor</th>
        <th>Value</th>
      </tr>
	  <xsl:for-each select='response/temperature/sensors/sensor'>
      <tr>
        <td> <xsl:value-of select='@name'/> </td>
        <td><xsl:value-of select='.'/> <xsl:value-of select='@unit'/></td>
      </tr>
	  </xsl:for-each>
    </table>
  </body>
  </html>
</xsl:template>
</xsl:stylesheet>

