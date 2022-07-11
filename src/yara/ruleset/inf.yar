rule inf
{
  strings:
    $re = /\xE2\x88\x9E|\xE2\x9C\x93/
  condition:
    $re
}
