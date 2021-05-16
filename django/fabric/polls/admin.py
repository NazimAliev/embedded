from django.contrib import admin
from .models import Polls 
from .models import Questions
from .models import Choices
from .models import Answers
from .models import AnswerText
from .models import AnswerOne
from .models import AnswerChoice

# Register your models here.
admin.site.register(Polls)
admin.site.register(Questions)
admin.site.register(Choices)
admin.site.register(Answers)
admin.site.register(AnswerText)
admin.site.register(AnswerOne)
admin.site.register(AnswerChoice)

class ItemAdmin(admin.ModelAdmin):
	exclude=("start_date",)
	readonly_fields=('Start_date', )
