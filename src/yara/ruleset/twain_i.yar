rule twain_nocase
{
  strings:
    $str = "Twain" nocase
  condition:
    $str
}
