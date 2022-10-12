rule huck
{
  strings:
    $re = /Huck[a-zA-Z]+|Saw[a-zA-Z]+/
  condition:
    $re
}
