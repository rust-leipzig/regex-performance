rule word
{
  strings:
    $re = /\b\w+nn\b/
  condition:
    $re
}
