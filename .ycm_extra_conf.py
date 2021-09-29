def Settings( **kwargs ):
  return {
    'flags': [ '-x', 'c', '-std=c17', '-W', '-Wall', '-DAPPNAME', '-DVERSION',
        '-I.'
        ],
  }
